## bin ##

import edizon
import zlib

CompressedSaveFileBuffer = edizon.getSaveFileBuffer() 
CompressedSaveFileBuffer = CompressedSaveFileBuffer[16:]
DecompressedSaveFileBuffer = zlib.decompress(bytes(CompressedSaveFileBuffer), 32 + 15)
saveFileBuffer = bytearray(DecompressedSaveFileBuffer)

def getValueFromSaveFile():
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	indirectAddress = int(strArgs[0], 16)
	address = int(strArgs[1], 16)
	addressSize = intArgs[0]
	valueSize = intArgs[1]
	
	offset = 0
	value = 0
		
	if indirectAddress != 0:
		for i in range(0, addressSize):
			offset = offset | (saveFileBuffer[indirectAddress + i] << i * 8)
		
	for i in range(0, valueSize):
		value = value | (saveFileBuffer[offset + address + i] << i * 8)
		
	return value

def setValueInSaveFile(value):
	global saveFileBuffer
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	indirectAddress = int(strArgs[0], 16)
	address = int(strArgs[1], 16)
	
	addressSize = intArgs[0]
	valueSize = intArgs[1]

	offset = 0
	
	if indirectAddress != 0:
		for i in range(0, addressSize):
			offset = offset | (saveFileBuffer[indirectAddress + i] << (i * 8))
		
	for i in range(0, valueSize):
		saveFileBuffer[offset + address + i] = (value & (0xFF << i * 8)) >> (i * 8)

def getModifiedSaveFile():
	header = [0x7E, 0xC0, 0xE2, 0xC9, 0x45, 0x98, 0xCE, 0x03, 0xA7, 0x98, 0xF1, 0x33, 0x6A, 0xE7, 0x25, 0x80]
	compressed_data = zlib.compress(saveFileBuffer, 9)
	print ("compressed")
	ready_data = bytearray(compressed_data)
	ready_data[0:0] = header
	
	return ready_data

