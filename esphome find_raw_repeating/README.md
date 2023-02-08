This is a easier way to find raw repeating codes in esphome without the log getting flooded with all the 433mhz noice

Just set the rx pin and board type in raw_repeating_433.yaml and upload the code, you will get 2 sliders in homeassistant, nSeparationLimit(idle) and tolerance (tolerance here are a number not % as in esphome Remote Receiver)

change the nSeparationLimit and mayby the tolerance if necessary untill you recieve the signal:



[07:12:23][D][diy raw:178]: size: 48 idle pulse 2289 pulses: [411, -1130, 1184,
-367, 404, -1137, 1184, -372, 1179, -379, 1170, -378, 1169, -394, 383, -1152, 39
5, -1151, 392, -1142, 1175, -391, 1154, -409, 364, -1156, 390, -1166, 384, -1159
, 1156, -403, 376, -1162, 1158, -396,
[07:12:23][D][diy raw:193]:   1153, -403, 1151, -412, 367, -1170, 379, -1164, 37
9, -1162, 383, -1161]
[07:12:23][D][diy raw:196]: signal done
[07:12:23][D][:197]:



remove the logg messages from the text untill you have this

[411, -1130, 1184,-367, 404, -1137, 1184, -372, 1179, -379, 1170, -378, 1169, -394, 383, -1152, 395, -1151, 392, -1142, 1175, -391, 1154, -409, 364, -1156, 390, -1166, 384, -1159, 1156, -403, 376, -1162, 1158, -396,1153, -403, 1151, -412, 367, -1170, 379, -1164, 379, -1162, 383, -1161]


this is what you use in esphome Remote Receiver

remember to set the idle: to a value less then idle pulse 2289 like 2ms, tolerance: 60% and filter: 100us should work ok


 
