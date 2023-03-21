
#break_in_det_image
gst-launch-1.0 filesrc location=../resource/data/test_walker1.jpeg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
flow_modelscope_pipeline task=domain-specific-object-detection id = damo/cv_tinynas_human-detection_damoyolo ! \
flow_python_extension input=../resource/config/break_in_count_deploy.yaml module=break_in_det_postprocess.py class= BreakInDetPostprocess function = postprocess ! \
videoconvert ! jpegenc ! filesink location=../resource/data/break_walk_res.jpg

#reid-person
gst-launch-1.0 filesrc location=../resource/data/image_reid_person.jpg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
flow_modelscope_pipeline task=image-reid-person id = damo/cv_passvitb_image-reid-person_market meta-key=model1 ! \
flow_metadata_aggregate name = mixer ! flow_python_extension module=reid_person_postprocess.py class= ReidPersonPostprocess ! fakesink \
filesrc location=../resource/data/image_reid_person.jpg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
flow_modelscope_pipeline task=image-reid-person id = damo/cv_passvitb_image-reid-person_market meta-key=model2 ! mixer.

#mot-count
gst-launch-1.0 filesrc location=../resource/data/test_walker1.jpeg ! decodebin ! videoconvert ! \
videoscale ! video/x-raw,format=RGB !  \
flow_modelscope_pipeline task=video-multi-object-tracking id = damo/cv_yolov5_video-multi-object-tracking_fairmot input=../resource/data/MOT17-03-partial.mp4 meta-key=mot ! \
flow_python_extension input =../resource/config/mot_counting_deploy.yaml module=mot_counting_postprocess.py class= MotCountingPostprocess ! \
videoconvert ! jpegenc ! filesink location=../resource/data/maas_test_detection_ok.jpg

#smoke-det-img
gst-launch-1.0 filesrc location=../resource/data/smoke_a388.jpg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
tee name=mytee \
mytee. ! queue ! flow_modelscope_pipeline task=domain-specific-object-detection id = damo/cv_tinynas_human-detection_damoyolo meta-key=human ! \
flow_metadata_aggregate name = mixer ! \
flow_python_extension input=../resource/config/smoke_det_deploy.yaml module=smoke_det_postprocess.py class= SmokeDetPostprocess ! \
videoconvert ! videoscale ! jpegenc ! filesink location=../resource/data/smoke_det_res.jpg  \
mytee. ! queue ! flow_modelscope_pipeline task=domain-specific-object-detection id = damo/cv_tinynas_object-detection_damoyolo_cigarette meta-key=cigare ! mixer.

#realtime detector
gst-launch-1.0 filesrc location=../resource/data/test_walker1.jpeg ! decodebin ! videoconvert ! \
videoscale ! video/x-raw,format=RGB !  \
flow_modelscope_pipeline task=image-object-detection id = damo/cv_cspnet_image-object-detection_yolox meta-key=detection ! \
flow_python_extension input=../resource/config/real_detector.yaml module=real_detector_vis.py class= RealDetectorPost function= postprocess ! \
videoconvert ! jpegenc ! filesink location=../resource/data/maas_test_detection_vis.jpg


