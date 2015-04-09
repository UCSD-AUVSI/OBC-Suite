
import json
import os

#-----------------------------------------------------------
#
def callback(data):
	print "received message from Heimdall: \"" + str(data) + "\""
	print "todo: handle messages from Heimdall"

	json_data = json.loads(data)
	command = json_data["command"]
	json_data = json_data["send"]

	if command == "start":
		ip_address = json_data["ip"]
		subnet = json_data["subnet"]
		folder = json_data["folder"]
		username = json_data["username"]
		password = json_data["password"]

		login_credentials = False
		while(not login_credentials):
			# run script 
			login_credentials=True

	if command == "upload_image":
		# try and upload image to team folder
		path = json_data["folder"]

		uploaded = False
		while(not uploaded):
			#try and upload the image at that path
			uploaded=True
