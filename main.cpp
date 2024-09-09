
#include <opencv2/opencv.hpp> 
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>
#include <string.h>
#include <stdio.h> 
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <iostream>

// Displays the File Meta Data
void imgMeta(std::string& file_path) {
    struct stat file_stat;

    // Use lstat to get file information
    if (lstat(file_path.c_str(), &file_stat) == -1) {
        perror("lstat");
        return;
    }

    std::cout << "File path: " << file_path << std::endl;
    std::cout << "File size: " << file_stat.st_size << " bytes" << std::endl;
}


// Function to scale an image to fit within a specified window size
cv::Mat scaleImageToFitWindow(const cv::Mat& image, int windowWidth, int windowHeight) {
    // Get the original image size
    int imageWidth = image.cols;
    int imageHeight = image.rows;

    // Calculate scaling factors
    double scaleX = static_cast<double>(windowWidth) / imageWidth;
    double scaleY = static_cast<double>(windowHeight) / imageHeight;
    double scale = std::min(scaleX, scaleY);

    // Create the affine transformation matrix for scaling
    cv::Mat affineMatrix = (cv::Mat_<double>(2, 3) << 
        scale, 0, 0, 
        0, scale, 0);

    // Create a Mat object to store the scaled image
    cv::Mat scaledImage;
    cv::warpAffine(image, scaledImage, affineMatrix, cv::Size(windowWidth, windowHeight), 
                   cv::INTER_LINEAR, cv::BORDER_CONSTANT);
    
    return scaledImage;
}



using namespace cv; 
int main(int argc, char** argv) 
{   
    // Variables and keys for parsing the command line.
    bool browsing=true;
    std::vector<String> files;
    const String keys = 
    "{help h usage ? |  | prints this message }"
    "{rows | 720 | Maximum number of rows for display window }"
    "{cols | 1080 | Maximum number of columns for display window }"
    "{dir | | Input Directory }";
    
    CommandLineParser parser(argc, argv, keys);
    parser.about("Image Browser by Kanaan Sullivan, v1.0.0");

    int numRows = parser.get<int>("rows");
    int numCols = parser.get<int>("cols");
    String path;

    if(parser.has("help")) {
        parser.printMessage();
        return 0;
    }
    if (parser.has("dir")) {
        path = parser.get<String>("dir");
    } else {
        parser.printMessage();
        return 0;
    }
    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }


    const char* dirPath = path.c_str();
    struct dirent *entry=nullptr;
    DIR *dir=nullptr;
    dir = opendir(dirPath);
    if (dir != nullptr) {
    while ((entry = readdir(dir)))
        // comapring d_type and DT_DIR because IS_DIR and lstat are not applicable when using Dirent
        if (entry->d_type != DT_DIR) {
            files.push_back(path + "/" + std::string(entry->d_name));
        }
        
    }

    // Removes non-image items from the vector
    for (auto i = files.begin(); i != files.end();
         ++i) {
        if (haveImageReader(*i) == 0) {
            files.erase(i);
            i--;
        }
    }


    // Iterator to act as a way to move through the images, while loop to keep image browser open until user quits.
    int i=0;
    while(browsing) {
        Mat img_color = imread(files[i] , IMREAD_COLOR);
        
        namedWindow ("display", WINDOW_NORMAL);
        resizeWindow("display", numCols, numRows);
        imshow("display", scaleImageToFitWindow(img_color,numCols,numRows));

        // display image info when one the image
        imgMeta(files[i]);
        std::cout<<"\nPress n for next image\np for previous image\nq to quit\n";

        int key = waitKeyEx(0);

        switch (key){
            case((int)('p')): 
                if (i == 0) {
                    i = 1;
                    break;
                } else {
                    i--;
                    break;
                }
            case((int)('q')):
                std::cout<<"QUITTING\n";
                browsing = false;
                break;
            case((int)('n')):
                if (i == files.size()-1) {
                        i = 0;
                        break;
                    } else {
                        i++;
                        break;
                }
        }
        destroyAllWindows();
    }
    closedir(dir);

    return 0; 
}

