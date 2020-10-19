// xnect.cpp : Defines the entry point for the console application.
//

#include "periphery/xnect/xnect.hpp"
#include "periphery/WebsocketServer.h"

#include <sys/timeb.h>
#include <ctime>
#include "domainvalue/Mode.h"
#include "domainobject/DelayPerData.h"
#include <iostream>
#include <chrono>
#include <asio/io_service.hpp>
#include <thread>

#define PORT_NUMBER 8080
#define SHOW_WINDOW 1
#define REPEAT_VIDEO 0
Mode mode = Mode::LIVE;

std::string images_to_load = "../../data/images";
std::string videoFilePath = "../../data/videos/pepper_front_1.mp4";
std::string recordingsFilePath = "./";

std::string currentDateTimeString() {
    time_t now = time(0);
    tm *ltm = localtime(&now);
    return std::to_string((1900 + ltm->tm_year)) + std::to_string(1 + ltm->tm_mon) + std::to_string(ltm->tm_mday) +
           "-" + std::to_string(1 + ltm->tm_hour) + std::to_string(1 + ltm->tm_min) + std::to_string(1 + ltm->tm_sec);
}

void
storeVectorToFile(std::vector<DelayPerData> data, const std::string &fileName = currentDateTimeString() + ".mock") {
    fstream recordingFile(recordingsFilePath + fileName, ios::out);

    if (!recordingFile.is_open()) {
        std::cerr << "Unable to create recording file. Missing write permissions?";
        return;
    }

    for (auto &i : data) {
        recordingFile << i.toString() + "\n";
    }

    recordingFile.close();
}

std::vector<DelayPerData> readFromFile(const std::string &fileNameWithPath) {
    fstream file(fileNameWithPath, ios::in);

    if (!file.is_open()) {
        std::cerr << "Cannot read file. Check if it exist or record a session first.";
    }

    std::vector<DelayPerData> data;

    std::string line;
    while (std::getline(file, line)) {
        DelayPerData singleData(line);
        data.push_back(singleData);
    }
    file.close();
    return data;
}

void sendDataToUnity(WebsocketServer &server, const std::string &data) {
    server.broadcastMessage(data);
}

void drawBones(cv::Mat &img, XNECT &xnect, int person) {
    //int numOfJoints = xnect.getNumOf3DJoints() - 2; // don't render feet, can be unstable
    int numOfJoints = xnect.getNumOf3DJoints();

    for (int i = 0; i < numOfJoints; i++) {
        int parentID = xnect.getJoint3DParent(i);
        if (parentID == -1) continue;
        // lookup 2 connected body/hand parts
        cv::Point2f partA = xnect.ProjectWithIntrinsics(xnect.getJoint3DIK(person, i));
        cv::Point2f partB = xnect.ProjectWithIntrinsics(xnect.getJoint3DIK(person, parentID));


        if (partA.x <= 0 || partA.y <= 0 || partB.x <= 0 || partB.y <= 0)
            continue;

        line(img, partA, partB, xnect.getPersonColor(person), 4);

    }

}

void drawJoints(cv::Mat &img, XNECT &xnect, int person) {

    //int numOfJoints = xnect.getNumOf3DJoints() - 2; // don't render feet, can be unstable
    int numOfJoints = xnect.getNumOf3DJoints();
    for (int i = 0; i < numOfJoints; i++) {
        int thickness = -1;
        int lineType = 8;
        cv::Point2f point2D = xnect.ProjectWithIntrinsics(xnect.getJoint3DIK(person, i));
        cv::circle(img, point2D, 6, xnect.getPersonColor(person), -1);

    }
}

void drawPeople(cv::Mat &img, XNECT &xnect) {
    for (int i = 0; i < xnect.getNumOfPeople(); i++)
        if (xnect.isPersonActive(i)) {
            drawBones(img, xnect, i);
            drawJoints(img, xnect, i);
        }

}

void writeTextOnImage(cv::Mat &frame, const std::string &text, const cv::Point &position) {
    cv::putText(frame, text, cv::Point(position.x + 1, position.y + 1), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 0, 0),
                1);
    cv::putText(frame, text, cv::Point(position.x, position.y), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(255, 255, 255),
                1);
}

void writeFPS(cv::Mat &frame, double time) {
    cv::Point position = cv::Point(10, 20);
    time = std::round(time * 100) / 100;
    std::string fpsText = "XNECT FPS: " + std::to_string(time);
    writeTextOnImage(frame, fpsText, position);
}

void writeCameraFPS(cv::Mat &frame, double time) {
    cv::Point position = cv::Point(10, frame.rows - 10);
    time = std::round(time * 100) / 100;
    std::string fpsText = "Camera FPS: " + std::to_string(time);
    writeTextOnImage(frame, fpsText, position);
}

void processImage(cv::Mat &frame, XNECT &xnect, WebsocketServer &server, bool showImage = true,
                  const std::string& windowName = "main", bool sendToUnity = true) {

    flip(frame, frame, 1);
    int frame_width = frame.cols;
    int frame_height = frame.rows;
    int64 start = cv::getTickCount();

    if (frame.empty()) return;

    xnect.processImg(frame);

    if (sendToUnity) {
        std::string data = xnect.getUnityData();
        sendDataToUnity(server, data);
    }
    drawPeople(frame, xnect);
    cv::resize(frame, frame, cv::Size(frame_width, frame_height), 0, 0, cv::INTER_LINEAR);
    int64 end = cv::getTickCount();
    double fps = cv::getTickFrequency() / (end - start);

    writeFPS(frame, fps);

    if (showImage) {
        cv::namedWindow(windowName);
        imshow(windowName, frame);
    }
}

bool playLive(XNECT &xnect, WebsocketServer &server) {
    cv::VideoCapture cap;

    if (!cap.open(0)) {
        std::cout << "Can't open webcam!\n";
        cv::waitKey(0);
        return false;
    }
    if (!(cap.set(CV_CAP_PROP_FRAME_WIDTH, xnect.processWidth) &&
          cap.set(CV_CAP_PROP_FRAME_HEIGHT, xnect.processHeight))) {

        std::cout << "[ ERROR ]: the connected webcam does not support " << xnect.processWidth << " x "
                  << xnect.processHeight << " resolution." << std::endl;
        cv::waitKey(0);
        return false;
    }
    // open the default camera, use something different from 0 otherwise;
    // Check VideoCapture documentation.

    for (;;) {

        cv::Mat frame;
        cap >> frame;

        processImage(frame, xnect, server, SHOW_WINDOW);

        char ch = cv::waitKey(1);

        if (ch == 27) break; // stop capturing by pressing ESC

        if (ch == 'p' || ch == 'P') {
            xnect.rescaleSkeletons();
            std::cout << "rescaling" << std::endl;

        }

        if (ch == 'r' || ch == 'R') {
            xnect.resetSkeletons();
            std::cout << "resetting" << std::endl;
        }


    }
    // the camera will be closed automatically upon exit

    return true;
}

void readVideoSeq(XNECT &xnect, WebsocketServer &server, const std::string& videoFilePath) {
    cv::VideoCapture cap(videoFilePath);

    if (!cap.isOpened()) {
        std::cout << "Error opening video file with path " << videoFilePath << ". Probably the file is missing?";
        return;
    }

    // Default resolution of the frame is obtained.The default resolution is system dependent.
    cv::Mat frame;
    cap >> frame;
    int frame_width = frame.cols;
    int frame_height = frame.rows;

    std::string fileName = currentDateTimeString() + "-pose_estimation_recording.avi";
    cv::VideoWriter video(recordingsFilePath + fileName, CV_FOURCC('M', 'J', 'P', 'G'), 10,
                          cv::Size(frame_width, frame_height));

    while (true) {

        cap >> frame;

        if (frame.empty()) {
            break;
        }

        processImage(frame, xnect, server, SHOW_WINDOW);
        writeCameraFPS(frame, cap.get(cv::CAP_PROP_FPS));
        video.write(frame);

        // Press  ESC on keyboard to exit
        char c = (char) cv::waitKey(25);
        if (c == 27)
            break;
    }
}

void recordSimulation(XNECT &xnect, WebsocketServer &server, const std::string& videoFilePath) {
    cv::VideoCapture cap(videoFilePath);

    if (!cap.isOpened()) {
        std::cout << "Error opening video file with path " << videoFilePath << ". Probably the file is missing?";
        return;
    }

    // Default resolution of the frame is obtained.The default resolution is system dependent.
    cv::Mat frame;
    cap >> frame;

    std::vector<DelayPerData> data;
    int frameIterator = 0;
    while (true) {
        std::cout << "Processing frame " + std::to_string(frameIterator) + "     \r";
        auto start = std::chrono::high_resolution_clock::now();

        cap >> frame;

        if (frame.empty()) {
            break;
        }

        processImage(frame, xnect, server, true);
        const std::string &dataString = xnect.getUnityData();
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
        DelayPerData newData(duration.count(), dataString);
        data.push_back(newData);

        // Press  ESC on keyboard to exit
        char c = (char) cv::waitKey(25);
        if (c == 27)
            break;

        frameIterator++;
    }

    // When everything done, release the video capture object
    cap.release();
    cv::destroyAllWindows();

    storeVectorToFile(data, "test.mock");
}

int main() {

    std::vector<DelayPerData> data = readFromFile("./test.mock");

    WebsocketServer server;

    XNECT xnect;
    server.message("rescaleSkeletons", [&xnect](const ClientConnection& conn, const std::string&)
    {
		std::cout << "Rescaling skeletons triggered by WebSocket client...\n";
		xnect.resetSkeletons();
		xnect.rescaleSkeletons();
    });

    //Start the networking thread
    std::thread serverThread([&server]() {
        server.run(PORT_NUMBER);
    });

	switch (mode) {
		case Mode::LIVE: {
			if (!playLive(xnect, server)) {
				return 1;
			}
			xnect.save_joint_positions(".");
			xnect.save_raw_joint_positions(".");
			break;
		}
		case Mode::VIDEOINPUT: {
			do {
				readVideoSeq(xnect, server, videoFilePath);
			} while (REPEAT_VIDEO);
			xnect.save_joint_positions(".");
			xnect.save_raw_joint_positions(".");
			break;
		}
		case Mode::SIMULATION_RECORDING: {
			recordSimulation(xnect, server, videoFilePath);
			break;
		}
	}

    return 0;
}

