// taken from https://github.com/CMU-Perceptual-Computing-Lab/caffe_rtpose/tree/master/src/rtpose

#ifndef MODEL_DESCRIPTOR_FACTORY_H
#define MODEL_DESCRIPTOR_FACTORY_H

#include <memory>
#include "modelDescriptor.h"

/**
 *  ModelDescriptorFactory is a factory class. It allows the creation of the different kind of ModelDescriptor (e.g. MPI_15, COCO_18).
 */
class ModelDescriptorFactory {
public:
    /** 
     * A class enum at which all the possible type of ModelDescriptor are included. In order to add a new ModelDescriptor,
     * include its name in this enum and add a new 'else if' statement inside ModelDescriptorFactory::createModelDescriptor().
     */
    enum class Type {
        MPI_15,
        COCO_18,
    };

    /**
       * The only function of this factory method. It creates the desired kind of ModelDescriptor (e.g. MPI_15, COCO_18).
       * @param type is a const ModelDescriptorFactory::Type component, specifying the type of model descriptor.
       * @param modelDescriptorUniquePtr is a std::unique_ptr<ModelDescriptor> object with the new created ModelDescriptor class.
       */
    const static void createModelDescriptor(const Type type, std::unique_ptr<ModelDescriptor> &modelDescriptorUniquePtr);
	int static connectLimbsCOCO(std::vector< std::vector<double>> &subset,
		std::vector< std::vector< std::vector<double> > > &connection,
		const float *heatmap_pointer,
		const float *in_peaks,
		int max_peaks,
		std::vector<float> &joints,
		ModelDescriptor *model_descriptor, int NET_RESOLUTION_HEIGHT, int NET_RESOLUTION_WIDTH, int DISPLAY_RESOLUTION_HEIGHT, int DISPLAY_RESOLUTION_WIDTH, int connect_min_subset_cnt, int connect_inter_threshold, int connect_inter_min_above_threshold, int connect_min_subset_score,int num_of_people);

};



#endif // MODEL_DESCRIPTOR_FACTORY_H
