// -----------------------------------------------------------------------------------------------
// Classname:
//      XNECT Implementation
//

#ifndef XNECT_implementation_H
#define XNECT_implementation_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <Eigen/Core>



struct XNECTImpl;

/** XNECT implementation class. */
class XNECT_implementation
{
	friend XNECTImpl;
	

public:
	//2D joints
	// { 0:'head', 1:'neck',  2:'Rsho',  3:'Relb',  4:'Rwri',  5:'Lsho',  6:'Lelb', 7:'Lwri', ...
	//   8:'Rhip', 9:'Rkne', 10:'Rank', 11:'Lhip', 12:'Lkne', 13:'Lank' }
	//3D joints
	// { 0:'head TOP', 1:'neck',  2:'Rsho',  3:'Relb',  4:'Rwri',  5:'Lsho',  6:'Lelb', 7:'Lwri', ...
	//   8:'Rhip', 9:'Rkne', 10:'Rank', 11:'Lhip', 12:'Lkne', 13:'Lank', 14: Root, 15: Spine, 16:'Head', 17: 'Rhand', 18: 'LHand', 19: 'Rfoot', 20: 'Lfoot' }

	XNECT_implementation(std::string configFile = "ConfigFile.params");
	/* image resolution before feeding into the network. Note: network resolution is different! Check config file for more.*/
	int processHeight, processWidth;
	/** Saves current raw positions(predicted 2D and 3D by CNN). Input: path to the folder. Folder should be created by a user.*/
	void save_raw_joint_positions(std::string file_path);
	/** Saves 2D and 3D after IK and filtering, final results.*/
	void save_joint_positions(std::string file_path);
	/** Set all parameters to default (including filtering variables).*/
	void resetSkeletons();
	/** Rescale skeletons based on raw 3d predictions.*/
	void rescaleSkeletons();
	/** Turn on/off IK.*/
	void toggleTracking();
	/** Get string of values that can be streamed to Unity.*/
	const std::string& getUnityData();
	/** Get 3D predictions from the network. Note: Prediction depth moved to correct depth, based on reprojection error.*/
	cv::Point3f getJoint3DPred(int person, int joint);
	/** Get 2D predictions from the network. Note: Prediction are filtered with 1 Euro filter. You can disable it in config file.*/
	cv::Point3f getJoint2DPred(int person, int joint);
	/** Get 3D after IK.*/
	cv::Point3f getJoint3DIK(int person, int joint);
	/** Get 2D after IK. 2D are reprojected from 3D IK*/
	cv::Point2f getJoint2DIK(int person, int joint);

	int getJoint3DParent(int joint);
	int getJoint2DParent(int joint);
	int getNumOf3DJoints();
	int getNumOf2DJoints();

	bool isPersonActive(int p);
	int getNumOfPeople();
	cv::Vec3b getPersonColor(int p);

	cv::Point2f ProjectWithIntrinsics(cv::Point3f point);

	/** Gets joint rotatation matrix 3x3. This can be used to drive a character in Unity for instance.*/
	const Eigen::Matrix<float, 3, 3>&  GetJointLocalRotation(int p, int joint);
	const Eigen::Matrix<float, 3, 3>&  GetSkeletonGlobalRotation(int p);
	const Eigen::Matrix<float, 3, 1>&  GetSkeletonGlobalPosition(int p); //basically joint 15
	const Eigen::Matrix<float, 3, 3>&  GetIntrinsics();

	virtual ~XNECT_implementation();

protected:

	int device_id;
	std::string NetProtoTxt_1, NetCaffeModel_1, NetProtoTxt_2, NetCaffeModel_2;
	std::string m_FeatureBlobName;
	int firstNetHeight, firstNetWidth, m_NumJoints;

	void setParameters(int numFeatSecondNet);

	int postCNNFirstNet(float *part_affinity_heatmaps, const float *peaks, int nms_layer_scale_w, int nms_layer_scale_h, cv::Size origImgSize);
	void preCNNSecondNet(float *input_data, const float *feature_data, int feature_h, int feature_w);
	void postCNNSecondNet(const float *output_3D, cv::Mat &m_ColorSized);

	XNECTImpl* impl;

};

#endif 
