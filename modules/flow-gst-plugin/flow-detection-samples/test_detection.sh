
#break_in_det_image
gst-launch-1.0 filesrc location=./data/test_walker1.jpeg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
mass_model task=domain-specific-object-detection id = damo/cv_tinynas_human-detection_damoyolo ! \
mass_model_post input=./data/break_in_count_deploy.yaml module=/Users/jingyao/Documents/23S1/AdaFlow/modules/flow-gst-plugin/flow-detection-samples/postprocess/break_in_det_postprocess.py ! \
videoconvert ! jpegenc ! filesink location=/Users/jingyao/Documents/23S1/test-img/res/break_walk.jpg

#break_in_det-video
gst-launch-1.0 filesrc location=/Users/jingyao/Documents/23S1/test-img/src/MOT17-03-partial.mp4 ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
mass_model task=domain-specific-object-detection id = damo/cv_tinynas_human-detection_damoyolo ! \
mass_model_post input=/Users/jingyao/Documents/23S1/test-img/src/break_in_count_deploy.yaml module=/Users/jingyao/Documents/23S1/AdaFlow/modules/flow-gst-plugin/flow-detection-samples/postprocess/break_in_det_postprocess.py ! \
videoconvert ! videoscale ! video/x-raw,format=I420 ! x264enc ! mp4mux ! filesink location=/Users/jingyao/Documents/23S1/test-img/res/tf_result.mp4