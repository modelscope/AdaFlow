
#break_in_det_image
gst-launch-1.0 filesrc location=/Users/jingyao/Documents/23S1/test-img/src/test_walker1.jpeg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
mass_model task=domain-specific-object-detection id = damo/cv_tinynas_human-detection_damoyolo ! \
mass_model_post input=/Users/jingyao/Documents/23S1/test-img/src/break_in_count_deploy.yaml module=/Users/jingyao/Documents/23S1/AdaFlow/modules/flow-gst-plugin/flow-detection-samples/postprocess/break_in_det_postprocess.py class= BreakInDetPostprocess function = postprocess ! \
videoconvert ! jpegenc ! filesink location=/Users/jingyao/Documents/23S1/test-img/res/break_walk.jpg

#break_in_det-video
gst-launch-1.0 filesrc location=/Users/jingyao/Documents/23S1/test-img/src/MOT17-03-partial.mp4 ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
mass_model task=domain-specific-object-detection id = damo/cv_tinynas_human-detection_damoyolo ! \
mass_model_post input=/Users/jingyao/Documents/23S1/test-img/src/break_in_count_deploy.yaml module=/Users/jingyao/Documents/23S1/AdaFlow/modules/flow-gst-plugin/flow-detection-samples/postprocess/break_in_det_postprocess.py class= BreakInDetPostprocess function = postprocess ! \
videoconvert ! videoscale ! video/x-raw,format=I420 ! x264enc ! mp4mux ! filesink location=/Users/jingyao/Documents/23S1/test-img/res/tf_result.mp4

#reid-person
gst-launch-1.0 filesrc location=/Users/jingyao/Documents/23S1/test-img/src/image_reid_person.jpg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
mass_model task=image-reid-person id = damo/cv_passvitb_image-reid-person_market meta-key=model1 ! \
meta_aggregator name = mixer ! mass_model_post input=/Users/jingyao/Documents/23S1/test-img/src/break_in_count_deploy.yaml module=/Users/jingyao/Documents/23S1/AdaFlow/modules/flow-python/adaflow-samples/detection/postprocess/reid_person_postprocess.py class= ReidPersonPostprocess ! fakesink \
filesrc location=/Users/jingyao/Documents/23S1/test-img/src/image_reid_person.jpg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
mass_model task=image-reid-person id = damo/cv_passvitb_image-reid-person_market meta-key=model2 ! mixer.