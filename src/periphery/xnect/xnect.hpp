// -----------------------------------------------------------------------------------------------
// Classname:
//      XNECT
//



#include "xnect_implementation.h"
#include <caffe/caffe.hpp>

#include "CWebSocketServer.hpp"

#include <modelDescriptor.cpp>
#include <modelDescriptorFactory.cpp>

CWEBSOCKETS_STATIC_DEFINITIONS

using namespace caffe;

#ifndef XNECT_H
#define XNECT_H


/** XNECT class. For more public functions looks up xnect_implementation.h and config file provided with example.*/
class XNECT: public XNECT_implementation
{
	
public:
	/** Constructor of the class, pass the path to the config file.*/
	XNECT(std::string configFile = "../../data/FullBodyTracker/");
	/** The main function for processing input image.*/
	void processImg(cv::Mat& img);
	void sendDataToUnity();

	virtual ~XNECT();

private:
	std::shared_ptr<caffe::Net<float>> m_FirstNet;
	std::shared_ptr<caffe::Net<float>> m_SecondNet;
	int m_NumChannels, m_NumFeatSecondNet,m_PersonOffset;
	Common::CWebSocketServer m_WSTransceiver;
	
	
	void WrapInputLayer(std::vector<cv::Mat>* input_channels_color,float *input_data);
	void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels_color);
};

void XNECT::sendDataToUnity()
{
	m_WSTransceiver.SendData(getUnityData());
}
XNECT::XNECT(std::string configFile ) : XNECT_implementation(configFile = "../../data/FullBodyTracker/")
{

	Caffe::set_mode(Caffe::GPU);

	Caffe::SetDevice(device_id);
	
	/* Load the networks */
	m_FirstNet.reset(new caffe::Net<float>(NetProtoTxt_1, caffe::Phase::TEST));
	m_FirstNet->CopyTrainedLayersFrom(NetCaffeModel_1);

	m_SecondNet.reset(new caffe::Net<float>(NetProtoTxt_2, caffe::Phase::TEST));
	m_SecondNet->CopyTrainedLayersFrom(NetCaffeModel_2);

	m_NumChannels = m_FirstNet->input_blobs()[0]->channels();
	
	const caffe::Blob<float> *input_layer, *output_layer;
	// check joint position net
	input_layer = m_FirstNet->input_blobs()[0];
	if (m_NumChannels != -1)
	{
		CHECK(m_NumChannels == input_layer->channels())
			<< "FirstNet: Input layer should have same number of channels as first net, namely " << m_NumChannels << ".";
	}
	else
		m_NumChannels = input_layer->channels();
	

	m_NumFeatSecondNet = 0;
	if (m_FirstNet->blob_by_name(m_FeatureBlobName) != nullptr)
		m_NumFeatSecondNet = m_FirstNet->blob_by_name(m_FeatureBlobName)->channels();
	
	setParameters(m_NumFeatSecondNet);

	struct cudaDeviceProp props;

	cudaGetDeviceProperties(&props, device_id);

	std::cout << "using GPU: " << props.name << std::endl;


	std::cout << "INFO: Attempting to started websocket server on 8080." << std::endl;;
	m_WSTransceiver.Initialize();
	m_WSTransceiver.StartServer("8080", "..");

}

void XNECT::WrapInputLayer(std::vector<cv::Mat>* input_channels_color,float *input_data)
{
	caffe::Blob<float>* input_layer = m_FirstNet->input_blobs()[0];

	// reshape nets
	input_layer->Reshape(1, m_NumChannels, firstNetHeight, firstNetWidth);
	m_FirstNet->Reshape();

	input_data = input_layer->mutable_cpu_data();
	input_channels_color->clear();
	for (int c = 0; c < m_NumChannels; ++c)
	{
		cv::Mat channel(firstNetHeight, firstNetWidth, CV_32FC1, input_data);
		input_channels_color->push_back(channel);
		input_data += firstNetWidth * firstNetHeight;
	}
	
}
void XNECT::Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels_color)
{
	/* Convert the input image to the input image format of the network. */
	/* This operation will write the separate BGR planes directly to the
	/* input layer of the network because it is wrapped by the cv::Mat
	/* objects in input_channels. */


	/* This operation will write the separate BGR planes directly to the
	* input layer of the network because it is wrapped by the cv::Mat
	* objects in input_channels. */
	cv::split(img, *input_channels_color);

	CHECK(reinterpret_cast<float*>(input_channels_color->at(0).data)
		== m_FirstNet->input_blobs()[0]->cpu_data())
		<< "Input channels are not wrapping the input layer of the joint position network.";

	//cudaDeviceSynchronize();
	//before = Common::getCurrentEpochTime();


}

void XNECT::processImg(cv::Mat& img)
{
	int m_NumCurrentPeople = 0;
	cv::Mat m_Color_float, m_ColorSized;
	cv::Size imgOrigSize = img.size();
	if (img.size() != cv::Size(processWidth, processHeight))
		cv::resize(img, img, cv::Size(processWidth, processHeight), CV_INTER_LINEAR);
	
	

	if (img.size() != cv::Size(firstNetWidth, firstNetHeight))
		cv::resize(img, m_ColorSized, cv::Size(firstNetWidth, firstNetHeight), CV_INTER_LINEAR);
	else	
		m_ColorSized = cv::Mat(img);
			
	m_ColorSized.convertTo(m_Color_float, CV_32FC3, 1.0f / 255.0f, -0.5f);

	caffe::Blob<float>* input_layer;
	float *input_data, *part_affinity_heatmaps;
	const float *peaks, *feature_data;
	std::vector<cv::Mat> input_channels_color, peaks_channels_color;

	WrapInputLayer(&input_channels_color,input_data);
	Preprocess(m_Color_float, &input_channels_color);

	m_FirstNet->Forward();
	
	part_affinity_heatmaps = m_FirstNet->blob_by_name("resized_map")->mutable_cpu_data();
	peaks = m_FirstNet->blob_by_name("joints")->mutable_cpu_data();

	int nms_layer_scale_w = firstNetWidth / m_FirstNet->blob_by_name("resized_map")->width();
	int nms_layer_scale_h = firstNetHeight / m_FirstNet->blob_by_name("resized_map")->height();
    m_NumCurrentPeople = postCNNFirstNet(part_affinity_heatmaps, peaks, nms_layer_scale_w, nms_layer_scale_h, imgOrigSize);
	

	if(m_NumCurrentPeople<1)
	   return;
	
	input_layer = m_SecondNet->input_blobs()[0];
	// reshape net

	input_layer->Reshape(m_NumCurrentPeople, m_NumFeatSecondNet + 3, m_NumJoints, 1); //  feature vector size 256 + u,v joint coordinate 

	m_SecondNet->Reshape();

	input_data = input_layer->mutable_cpu_data();
	feature_data = m_FirstNet->blob_by_name(m_FeatureBlobName)->cpu_data();
	
	
	preCNNSecondNet(input_data, feature_data, m_FirstNet->blob_by_name(m_FeatureBlobName)->height(), m_FirstNet->blob_by_name(m_FeatureBlobName)->width());
	
	m_SecondNet->Forward();
	
	postCNNSecondNet(m_SecondNet->blob_by_name("pred_3d_pose_scale_root")->cpu_data(), m_ColorSized);




}

XNECT::~XNECT()
{
	
}


#endif 