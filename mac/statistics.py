#!/usr/bin/env python3

import os
from os.path import isfile, join
import subprocess
import shutil
import json


### Flask
from flask import (
	Flask,
	request,
	render_template,
	send_from_directory,
	send_file,
	jsonify
)
### SQLAlchemy
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy import Column, Integer, String, Float, ForeignKey
from sqlalchemy.orm import relationship
# import psycopg2


app = Flask(__name__)
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
# app.config['SQLALCHEMY_DATABASE_URI'] = 'postgresql://localhost/hands'
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:////Volumes/iOS/dip/Results/hands.db'
db = SQLAlchemy(app)

class Entry(db.Model):
	id = Column(Integer, primary_key=True)

	originalPath = Column(String(100))
	colorFilterPath = Column(String(100))
	sizeFilterPath = Column(String(100))
	redOverlayPath = Column(String(100))
	contourCount = Column(Integer)
	handCount = Column(Integer)

	hands = relationship("Hand", back_populates="entry")

	def __init__(self, originalPath, colorFilterPath, sizeFilterPath, redOverlayPath, contourCount, handCount):
		self.originalPath = originalPath
		self.colorFilterPath = colorFilterPath
		self.sizeFilterPath = sizeFilterPath
		self.redOverlayPath = redOverlayPath
		self.contourCount = contourCount
		self.handCount = handCount

	def __repr__(self):
		return f"<Entry '{self.originalPath}' '{self.colorFilterPath}' '{self.sizeFilterPath}' '{self.redOverlayPath}' c:{self.contourCount} h:{self.handCount}>"

	def serialize(self):
		return {
			"originalPath": self.originalPath,
			"colorFilterPath": self.colorFilterPath,
			"sizeFilterPath": self.sizeFilterPath,
			"redOverlayPath": self.redOverlayPath,
			"contourCount": self.contourCount,
			"handCount": self.handCount,
			"hands": [x.serialize() for x in self.hands],
		}

class Hand(db.Model):
	id = Column(Integer, primary_key=True)

	fingers = Column(Integer)
	orientation = Column(Integer)
	controlAngle = Column(Float)
	size = Column(Float)
	center_x = Column(Integer)
	center_y = Column(Integer)
	thumbTip_x = Column(Integer)
	thumbTip_y = Column(Integer)
	indexTip_x = Column(Integer)
	indexTip_y = Column(Integer)

	entry_id = Column(Integer, ForeignKey('entry.id'))
	entry = relationship("Entry", back_populates="hands")

	def __init__(self, fingers, orientation, controlAngle, size, center, thumbTip, indexTip):
		self.fingers = fingers
		self.orientation = orientation
		self.controlAngle = controlAngle
		self.size = size
		self.center_x, self.center_y = center
		self.thumbTip_x, self.thumbTip_y = thumbTip
		self.indexTip_x, self.indexTip_y = indexTip

	def serialize(self):
		return {
			"fingers": self.fingers,
			"orientation": self.orientation,
			"controlAngle": self.controlAngle,
			"size": self.size,
			"center_x": self.center_x,
			"center_y": self.center_y,
			"thumbTip_x": self.thumbTip_x,
			"thumbTip_y": self.thumbTip_y,
			"indexTip_x": self.indexTip_x,
			"indexTip_y": self.indexTip_y,
		}

imagePath = "../Hands"
resultsPath = "../Results"


@app.after_request
def set_response_headers(response):
	response.headers['Cache-Control'] = 'no-cache, no-store, must-revalidate'
	response.headers['Pragma'] = 'no-cache'
	response.headers['Expires'] = '0'
	return response

@app.route('/')
def homepage():
	return app.send_static_file('index.html')

@app.route('/hands')
def hands():
	return app.send_static_file('hands.html')

@app.route('/entries')
def json_entries():
	entries = db.session.query(Entry).all()
	return jsonify({
		'entries': [x.serialize() for x in entries]
	})

@app.route('/json/hands')
def json_hands():
	entries = db.session.query(Entry).filter(Entry.handCount > 0).all()
	return jsonify({
		'entries': [x.serialize() for x in entries]
	})

def main():
	# subprocess.call(['dropdb', 'hands'])
	# subprocess.call(['createdb', 'hands'])
	subprocess.call(['rm', '../Results/hands.db'])
	db.create_all()
	try:
		db.session.commit()
	except:
		print("> E: An exception was thrown when commiting schemas")

	files = [f for f in os.listdir(imagePath) if isfile(join(imagePath, f))]
	hands = 0
	# files = files[:30]
	for idx, file in enumerate(files):
		filePath = join(imagePath, file)
		imageNumber = file[5:][:-4]

		resultFilePath = join(resultsPath, f"{imageNumber}_0.jpg")
		colorFileFilePath = join(resultsPath, f"{imageNumber}_1.jpg")
		sizeFileFilePath = join(resultsPath, f"{imageNumber}_2.jpg")
		redOverlayFilePath = join(resultsPath, f"{imageNumber}_3.jpg")

		cp = subprocess.run(['.theos/obj/macosx/debug/mac', filePath], capture_output=True)

		shutil.copy('_original.jpg', resultFilePath)
		shutil.copy('_filter_color.jpg', colorFileFilePath)
		shutil.copy('_filter_size.jpg', sizeFileFilePath)
		shutil.copy('_red_overlay.jpg', redOverlayFilePath)

		results = json.loads(cp.stdout)
		# print(results)
		# break
		# entry = db.session.query(Entry).filter(Entry.originalPath == f"{imageNumber}_0.jpg").one_or_none()
		# if entry is None:
		entry = Entry(f"{imageNumber}_0.jpg", f"{imageNumber}_1.jpg", f"{imageNumber}_2.jpg", f"{imageNumber}_3.jpg", results['c'], results['h'])
		db.session.add(entry)
		for hand in results['hands']:
			hand = Hand(hand['fingers'], hand['orientation'],
				hand['controlAngle'], hand['size'], hand['center'].split(','),
				hand['thumbTip'].split(','), hand['indexTip'].split(',')
				)
			hand.entry = entry
			db.session.add(hand)

		hands += results['h']
		print(f"\rProcessed {100 * (idx + 1)/len(files):05.3f}%", end='', flush=True)
	print()

	db.session.flush()
	db.session.commit()

	print(f"Found {hands} hands in {len(files)}")


def results():
	entries = db.session.query(Entry).all()
	hands = db.session.query(Entry).filter(Entry.handCount > 0).all()
	print(f"{len(hands)}/{len(entries)} ({100 * len(hands)/len(entries):05.3f}%)")
	return
	# print('\n'.join([h.__repr__() for h in hands]))
	for hand in hands:
		cp = subprocess.run(['.theos/obj/macosx/debug/validate_image', join(resultsPath, hand.redOverlayPath)], capture_output=True)
		output = int(cp.stdout[:-1].decode("utf-8"), 16)
		outputChar = chr(output)
		if output == 27:
			break

if __name__ == '__main__':
	# main()
	# app.run(use_reloader=True, host='0.0.0.0', port=8080)
	results()
