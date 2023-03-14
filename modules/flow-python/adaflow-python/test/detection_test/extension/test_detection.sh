
#break_in_det_image
gst-launch-1.0 filesrc location=../resource/data/test_walker1.jpeg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
maas_model task=domain-specific-object-detection id = damo/cv_tinynas_human-detection_damoyolo ! \
maas_model_post input=../resource/config/break_in_count_deploy.yaml module=break_in_det_postprocess.py class= BreakInDetPostprocess function = postprocess ! \
videoconvert ! jpegenc ! filesink location=../resource/data/break_walk_res.jpg

#reid-person
gst-launch-1.0 filesrc location=../resource/data/image_reid_person.jpg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
maas_model task=image-reid-person id = damo/cv_passvitb_image-reid-person_market meta-key=model1 ! \
meta_aggregator name = mixer ! maas_model_post module=reid_person_postprocess.py class= ReidPersonPostprocess ! fakesink \
filesrc location=../resource/data/image_reid_person.jpg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
maas_model task=image-reid-person id = damo/cv_passvitb_image-reid-person_market meta-key=model2 ! mixer.

#mot-count
gst-launch-1.0 filesrc location=../resource/data/test_walker1.jpeg ! decodebin ! videoconvert ! \
videoscale ! video/x-raw,format=RGB !  \
maas_model task=video-multi-object-tracking id = damo/cv_yolov5_video-multi-object-tracking_fairmot input=../resource/data/MOT17-03-partial.mp4 meta-key=mot ! \
maas_model_post input =../resource/config/mot_counting_deploy.yaml module=mot_counting_postprocess.py class= MotCountingPostprocess ! \
videoconvert ! jpegenc ! filesink location=../resource/data/maas_test_detection_ok.jpg

#smoke-det-img
gst-launch-1.0 filesrc location=../resource/data/smoke_a388.jpg ! \
decodebin ! videoconvert ! videoscale ! video/x-raw,format=RGB ! \
tee name=mytee \
mytee. ! queue ! maas_model task=domain-specific-object-detection id = damo/cv_tinynas_human-detection_damoyolo meta-key=human ! \
meta_aggregator name = mixer ! \
maas_model_post input=../resource/config/smoke_det_deploy.yaml module=smoke_det_postprocess.py class= SmokeDetPostprocess ! \
videoconvert ! videoscale ! jpegenc ! filesink location=../resource/data/smoke_det_res.jpg  \
mytee. ! queue ! maas_model task=domain-specific-object-detection id = damo/cv_tinynas_object-detection_damoyolo_cigarette meta-key=cigare ! mixer.
