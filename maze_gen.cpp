#include "app.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>
#include <algorithm>

// Secure access to map
uchar App::getmap(cv::Mat& map, int x, int y)
{
    x = std::clamp(x, 0, map.cols - 1);
    y = std::clamp(y, 0, map.rows - 1);
    return map.at<uchar>(y, x);
}

// Random map gen – returns start position
cv::Point App::genLabyrinth(cv::Mat& map) {
    const int rows = map.rows;
    const int cols = map.cols;

    map.setTo('#'); // celé bludištì jako zdi

    std::vector<std::pair<int, int>> stack;
    stack.emplace_back(1, 1);
    map.at<uchar>(1, 1) = '.'; // poèáteèní bod

    std::random_device rd;
    std::mt19937 rng(rd());

    auto neighbors = [&](int x, int y) {
        std::vector<std::tuple<int, int, int, int>> result;
        if (x > 2) result.emplace_back(x - 2, y, x - 1, y);
        if (y > 2) result.emplace_back(x, y - 2, x, y - 1);
        if (x < cols - 3) result.emplace_back(x + 2, y, x + 1, y);
        if (y < rows - 3) result.emplace_back(x, y + 2, x, y + 1);
        std::shuffle(result.begin(), result.end(), rng);
        return result;
        };

    while (!stack.empty()) {
        auto [x, y] = stack.back();
        auto ns = neighbors(x, y);

        bool carved = false;
        for (auto [nx, ny, mx, my] : ns) {
            if (map.at<uchar>(ny, nx) == '#') {
                map.at<uchar>(my, mx) = '.';
                map.at<uchar>(ny, nx) = '.';
                stack.emplace_back(nx, ny);
                carved = true;
                break;
            }
        }

        if (!carved) {
            stack.pop_back();
        }
    }

    // Nastavíme start a cíl
    map.at<uchar>(1, 1) = 'X'; // start
    map.at<uchar>(rows - 2, cols - 2) = 'e'; // end

    std::cout << "[Maze]\n";
    for (int j = 0; j < rows; ++j) {
        for (int i = 0; i < cols; ++i) {
            std::cout << static_cast<char>(map.at<uchar>(j, i));
        }
        std::cout << "\n";
    }

    return { 1, 1 }; // vrátíme souøadnice startu
}
