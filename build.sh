rm -rf build
cmake -S . -B build
cmake --build build

# select sink
./build/v8_pose_bytetrack_RTSPfile file:///app/Robin/v8_pose_bytetrack_file/cxk.mp4
# ./build/v8_pose_bytetrack_sinkfile file:///app/Robin/v8_pose_bytetrack_file/cxk.mp4
