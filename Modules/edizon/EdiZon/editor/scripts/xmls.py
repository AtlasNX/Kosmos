import edizon
from xmltodict import parse, unparse
from functools import reduce
import operator

saveFileBuffer = parse(edizon.getSaveFileString())

def getFromDict(dataDict, mapList):
    return reduce(operator.getitem, mapList, dataDict)
	
def setInDict(dataDict, mapList, value):
    getFromDict(dataDict, mapList[:-1])[mapList[-1]] = value
	
def getValueFromSaveFile():
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	
	return int(getFromDict(saveFileBuffer, strArgs)) if intArgs[0] == 0 else (1 if getFromDict(saveFileBuffer, strArgs) == "true" else 0)
		
def setValueInSaveFile(value):
	global saveFileBuffer
	
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	
	setInDict(saveFileBuffer, strArgs, value if intArgs[0] == 0 else ("false" if value == 0 else "true"))
	
def getStringFromSaveFile():
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	
	return str(getFromDict(saveFileBuffer, strArgs))

def setStringInSaveFile(value):
	global saveFileBuffer
	
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	
	setInDict(saveFileBuffer, strArgs, value)
	
def getModifiedSaveFile():
	return bytearray(unparse(saveFileBuffer), 'utf-8')