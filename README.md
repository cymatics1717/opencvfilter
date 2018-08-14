# 测试用命令   
```
export GST_PLUGIN_PATH=.:$GST_PLUGIN_PATH
export GST_DEBUG=*:4;
gst-launch-1.0 v4l2src device=/dev/video1 ! video/x-raw,width=800,height=600,framerate=30/1 ! videoconvert ! calib ! videoconvert ! fpsdisplaysink
```
gstreamer 加载filter 会递归的搜索GST_PLUGIN_PATH的所有子目录．

# 说明

经过测试，在Ubuntu 16.04上，除了fpsdisplaysink，可显示的sink还有如下这么多:  

autovideosink  
gtksink  
glimagesink  
vaapisink  
clutterautovideosink  
aasink  
cacasink  
xvimagesink  
ximagesink  
waylandsink [weston]  

intervideosink  
fdsink  
checksumsink  
shmsink/shmsrc  

