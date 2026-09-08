#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cstdint>

inline std::vector<uint8_t> load_raw_volume(const std::string& filepath, size_t total_voxels) {
    std::vector<uint8_t> volume_data(total_voxels);
    std::ifstream file(filepath, std::ios::binary);

    if (file.is_open()) {
        file.read(reinterpret_cast<char*>(volume_data.data()), total_voxels);
        file.close();
        std::cout << "Successfully loaded: " << filepath << std::endl;
    }
    else {
        std::cerr << "Failed to open raw volume file at: " << filepath << std::endl;
    }

    return volume_data;
}