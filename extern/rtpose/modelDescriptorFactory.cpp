// code for function connectLimbsCOCO taken from https://github.com/CMU-Perceptual-Computing-Lab/caffe_rtpose/blob/a4778bb1c3eb74d7250402016047216f77b4dba6/examples/rtpose/rtpose.cpp
// the rest https://github.com/CMU-Perceptual-Computing-Lab/caffe_rtpose/tree/master/src/rtpose
#include "modelDescriptorFactory.h"
#include <stdexcept>

struct ColumnCompare
{
	bool operator()(const std::vector<double>& lhs, const std::vector<double>& rhs) const
	{
		return lhs[2] > rhs[2];
		//return lhs[0] > rhs[0];
	}
};

const void ModelDescriptorFactory::createModelDescriptor(const ModelDescriptorFactory::Type type, std::unique_ptr<ModelDescriptor> &modelDescriptorUniquePtr)
{
    if (type == Type::MPI_15)
    {
        modelDescriptorUniquePtr.reset(new ModelDescriptor{
            {{0,  "Head"},
             {1,  "Neck"},
             {2,  "RShoulder"},
             {3,  "RElbow"},
             {4,  "RWrist"},
             {5,  "LShoulder"},
             {6,  "LElbow"},
             {7,  "LWrist"},
             {8,  "RHip"},
             {9,  "RKnee"},
             {10, "RAnkle"},
             {11, "LHip"},
             {12, "LKnee"},
             {13, "LAnkle"},
             {14, "Chest"},
             {15, "Bkg"}},                                                                                                      // partToName
            {0,1, 1,2, 2,3, 3,4, 1,5, 5,6, 6,7, 1,14, 14,11, 11,12, 12,13, 14,8, 8,9, 9,10},                                    // limbSequence
            {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 38, 39, 40, 41, 42, 43, 32, 33, 34, 35, 36, 37}    // mapIdx
        });
    }

    else if (type == Type::COCO_18)
    {
        modelDescriptorUniquePtr.reset(new ModelDescriptor{
            {{0,  "Nose"},
             {1,  "Neck"},
             {2,  "RShoulder"},
             {3,  "RElbow"},
             {4,  "RWrist"},
             {5,  "LShoulder"},
             {6,  "LElbow"},
             {7,  "LWrist"},
             {8,  "RHip"},
             {9,  "RKnee"},
             {10, "RAnkle"},
             {11, "LHip"},
             {12, "LKnee"},
             {13, "LAnkle"},
             {14, "REye"},
             {15, "LEye"},
             {16, "REar"},
             {17, "LEar"},
             {18, "Bkg"}},                                                                                                                          // partToName
            { 1,2,   1,5,   2,3,   3,4,   5,6,   6,7,   1,8,   8,9,   9,10, 1,11,  11,12, 12,13,  1,0,   0,14, 14,16,  0,15, 15,17, 2,16,  5,17},   // limbSequence
            {31,32, 39,40, 33,34, 35,36, 41,42, 43,44, 19,20, 21,22, 23,24, 25,26, 27,28, 29,30, 47,48, 49,50, 53,54, 51,52, 55,56, 37,38, 45,46}   // mapIdx
        });
    }

    else
    {
        throw std::runtime_error{std::string{"Undefined ModelDescriptor selected."}};
    }
}

int  ModelDescriptorFactory::connectLimbsCOCO(
std::vector< std::vector<double>> &subset,
std::vector< std::vector< std::vector<double> > > &connection,
const float *heatmap_pointer,
const float *in_peaks,
int max_peaks,
std::vector<float> &joints,
ModelDescriptor *model_descriptor, int NET_RESOLUTION_HEIGHT, int NET_RESOLUTION_WIDTH, int DISPLAY_RESOLUTION_HEIGHT, int DISPLAY_RESOLUTION_WIDTH, int connect_min_subset_cnt, int connect_inter_threshold, int connect_inter_min_above_threshold, int connect_min_subset_score, int num_of_people) {

	

	/* Parts Connection ---------------------------------------*/
	const auto num_parts = model_descriptor->get_number_parts();
	const auto limbSeq = model_descriptor->get_limb_sequence();
	const auto mapIdx = model_descriptor->get_map_idx();
	const auto number_limb_seq = model_descriptor->number_limb_sequence();

	//CHECK_EQ(num_parts, 18) << "Wrong connection function for model";
	//CHECK_EQ(number_limb_seq, 19) << "Wrong connection function for model";

	int SUBSET_CNT = num_parts + 2;
	int SUBSET_SCORE = num_parts + 1;
	int SUBSET_SIZE = num_parts + 3;

	const int peaks_offset = 3 * (max_peaks + 1);

	const float *peaks = in_peaks;
	subset.clear();
	connection.clear();

	for (int k = 0; k < number_limb_seq; k++) {
		const float* map_x = heatmap_pointer + mapIdx[2 * k] * NET_RESOLUTION_HEIGHT * NET_RESOLUTION_WIDTH;
		const float* map_y = heatmap_pointer + mapIdx[2 * k + 1] * NET_RESOLUTION_HEIGHT * NET_RESOLUTION_WIDTH;

		const float* candA = peaks + limbSeq[2 * k] * peaks_offset;
		const float* candB = peaks + limbSeq[2 * k + 1] * peaks_offset;

		std::vector< std::vector<double> > connection_k;
		int nA = candA[0];
		int nB = candB[0];

		// add parts into the subset in special case
		if (nA == 0 && nB == 0) {
			continue;
		}
		else if (nA == 0) {
			for (int i = 1; i <= nB; i++) {
				int num = 0;
				int indexB = limbSeq[2 * k + 1];
				for (int j = 0; j < subset.size(); j++) {
					int off = limbSeq[2 * k + 1] * peaks_offset + i * 3 + 2;
					if (subset[j][indexB] == off) {
						num = num + 1;
						continue;
					}
				}
				if (num != 0) {
					//LOG(INFO) << " else if (nA==0) shouldn't have any nB already assigned?";
				}
				else {
					std::vector<double> row_vec(SUBSET_SIZE, 0);
					row_vec[limbSeq[2 * k + 1]] = limbSeq[2 * k + 1] * peaks_offset + i * 3 + 2; //store the index
					row_vec[SUBSET_CNT] = 1; //last number in each row is the parts number of that person
					row_vec[SUBSET_SCORE] = candB[i * 3 + 2]; //second last number in each row is the total score
					subset.push_back(row_vec);
				}
				//LOG(INFO) << "nA==0 New subset on part " << k << " subsets: " << subset.size();
			}
			continue;
		}
		else if (nB == 0) {
			for (int i = 1; i <= nA; i++) {
				int num = 0;
				int indexA = limbSeq[2 * k];
				for (int j = 0; j < subset.size(); j++) {
					int off = limbSeq[2 * k] * peaks_offset + i * 3 + 2;
					if (subset[j][indexA] == off) {
						num = num + 1;
						continue;
					}
				}
				if (num == 0) {
					std::vector<double> row_vec(SUBSET_SIZE, 0);
					row_vec[limbSeq[2 * k]] = limbSeq[2 * k] * peaks_offset + i * 3 + 2; //store the index
					row_vec[SUBSET_CNT] = 1; //last number in each row is the parts number of that person
					row_vec[SUBSET_SCORE] = candA[i * 3 + 2]; //second last number in each row is the total score
					subset.push_back(row_vec);
					//LOG(INFO) << "nB==0 New subset on part " << k << " subsets: " << subset.size();
				}
				else {
					//LOG(INFO) << "nB==0 discarded would have added";
				}
			}
			continue;
		}

		std::vector< std::vector<double>> temp;
		const int num_inter = 10;

		for (int i = 1; i <= nA; i++) {
			for (int j = 1; j <= nB; j++) {
				float s_x = candA[i * 3];
				float s_y = candA[i * 3 + 1];
				float d_x = candB[j * 3] - candA[i * 3];
				float d_y = candB[j * 3 + 1] - candA[i * 3 + 1];
	
				float norm_vec = sqrt(d_x*d_x + d_y*d_y);
				if (norm_vec<1e-6) {
					// The peaks are coincident. Don't connect them.
					continue;
				}
				float vec_x = d_x / norm_vec;
				float vec_y = d_y / norm_vec;

				float sum = 0;
				int count = 0;

				for (int lm = 0; lm < num_inter; lm++) {
					int my = round(s_y + lm*d_y / num_inter);
					int mx = round(s_x + lm*d_x / num_inter);
					if (mx >= NET_RESOLUTION_WIDTH) {
						//LOG(ERROR) << "mx " << mx << "out of range";
						mx = NET_RESOLUTION_WIDTH - 1;
					}
					if (my >= NET_RESOLUTION_HEIGHT) {
						//LOG(ERROR) << "my " << my << "out of range";
						my = NET_RESOLUTION_HEIGHT - 1;
					}
					//CHECK_GE(mx, 0);
					//CHECK_GE(my, 0);
					int idx = my * NET_RESOLUTION_WIDTH + mx;
					float score = (vec_x*map_x[idx] + vec_y*map_y[idx]);
					if (score >  connect_inter_threshold) {
						sum = sum + score;
						count++;
					}
				}
				//float score = sum / count; // + std::min((130/dist-1),0.f)

				if (count >  connect_inter_min_above_threshold) {//num_inter*0.8) { //thre/2
					// parts score + cpnnection score
					std::vector<double> row_vec(4, 0);
					row_vec[3] = sum / count + candA[i * 3 + 2] + candB[j * 3 + 2]; //score_all
					row_vec[2] = sum / count;
					row_vec[0] = i;
					row_vec[1] = j;
					temp.push_back(row_vec);
				}
			}
		}

		//** select the top num connection, assuming that each part occur only once
		// sort rows in descending order based on parts + connection score
		if (temp.size() > 0)
			std::sort(temp.begin(), temp.end(), ColumnCompare());

		int num = std::min(nA, nB);
		int cnt = 0;
		std::vector<int> occurA(nA, 0);
		std::vector<int> occurB(nB, 0);

		
		for (int row = 0; row < temp.size(); row++) {
			if (cnt == num) {
				break;
			}
			else{
				int i = int(temp[row][0]);
				int j = int(temp[row][1]);
				float score = temp[row][2];
				if (occurA[i - 1] == 0 && occurB[j - 1] == 0) { // && score> (1+thre)
					std::vector<double> row_vec(3, 0);
					row_vec[0] = limbSeq[2 * k] * peaks_offset + i * 3 + 2;
					row_vec[1] = limbSeq[2 * k + 1] * peaks_offset + j * 3 + 2;
					row_vec[2] = score;
					connection_k.push_back(row_vec);
					cnt = cnt + 1;
					occurA[i - 1] = 1;
					occurB[j - 1] = 1;
				}
			}
		}

		//** cluster all the joints candidates into subset based on the part connection
		// initialize first body part connection 15&16
		if (k == 0) {
			std::vector<double> row_vec(num_parts + 3, 0);
			for (int i = 0; i < connection_k.size(); i++) {
				double indexB = connection_k[i][1];
				double indexA = connection_k[i][0];
				row_vec[limbSeq[0]] = indexA;
				row_vec[limbSeq[1]] = indexB;
				row_vec[SUBSET_CNT] = 2;
				// add the score of parts and the connection
				row_vec[SUBSET_SCORE] = peaks[int(indexA)] + peaks[int(indexB)] + connection_k[i][2];
				//LOG(INFO) << "New subset on part " << k << " subsets: " << subset.size();
				subset.push_back(row_vec);
			}
		}else{
			if (connection_k.size() == 0) {
				continue;
			}

			// A is already in the subset, find its connection B
			for (int i = 0; i < connection_k.size(); i++) {
				int num = 0;
				double indexA = connection_k[i][0];
				double indexB = connection_k[i][1];

				for (int j = 0; j < subset.size(); j++) {
					if (subset[j][limbSeq[2 * k]] == indexA) {
						subset[j][limbSeq[2 * k + 1]] = indexB;
						num = num + 1;
						subset[j][SUBSET_CNT] = subset[j][SUBSET_CNT] + 1;
						subset[j][SUBSET_SCORE] = subset[j][SUBSET_SCORE] + peaks[int(indexB)] + connection_k[i][2];
					}
				}
				// if can not find partA in the subset, create a new subset
				if (num == 0) {
					//LOG(INFO) << "New subset on part " << k << " subsets: " << subset.size();
					std::vector<double> row_vec(SUBSET_SIZE, 0);
					row_vec[limbSeq[2 * k]] = indexA;
					row_vec[limbSeq[2 * k + 1]] = indexB;
					row_vec[SUBSET_CNT] = 2;
					row_vec[SUBSET_SCORE] = peaks[int(indexA)] + peaks[int(indexB)] + connection_k[i][2];
					subset.push_back(row_vec);
				}
			}
		}
	}

	//** joints by deleteing some rows of subset which has few parts occur
	int cnt = 0;

	for (int i = 0; i < subset.size(); i++) {
		//if (subset[i][SUBSET_CNT]<1) {
		//	LOG(INFO) << "BAD SUBSET_CNT";
		//}
		if (subset[i][SUBSET_CNT] >= connect_min_subset_cnt && (subset[i][SUBSET_SCORE] / subset[i][SUBSET_CNT])> connect_min_subset_score) {
			for (int j = 0; j < num_parts; j++) {
				int idx = int(subset[i][j]);
				if (idx) {
					joints[cnt*num_parts * 3 + j * 3 + 2] = peaks[idx];
					joints[cnt*num_parts * 3 + j * 3 + 1] = peaks[idx - 1] * DISPLAY_RESOLUTION_HEIGHT / (float)NET_RESOLUTION_HEIGHT;//(peaks[idx-1] - padh) * ratio_h;
					joints[cnt*num_parts * 3 + j * 3] = peaks[idx - 2] * DISPLAY_RESOLUTION_WIDTH / (float)NET_RESOLUTION_WIDTH;//(peaks[idx-2] -padw) * ratio_w;
				}
				else{
					joints[cnt*num_parts * 3 + j * 3 + 2] = 0;
					joints[cnt*num_parts * 3 + j * 3 + 1] = 0;
					joints[cnt*num_parts * 3 + j * 3] = 0;
				}
			}
			cnt++;
			if (cnt == num_of_people) break;
		}
	}

	return cnt;
}




