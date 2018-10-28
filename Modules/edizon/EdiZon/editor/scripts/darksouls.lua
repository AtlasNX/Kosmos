-- darksoulsremastered (modified bin.lua) --

md5 = require("lib.md5")

saveFileBuffer = edizon.getSaveFileBuffer()

function getValueFromSaveFile()
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	indirectAddress = tonumber(strArgs[1], 16)
	address = tonumber(strArgs[2], 16)
	addressSize = intArgs[1]
	valueSize = intArgs[2]
	
	offset = 0
	value = 0
		
	if indirectAddress ~= 0 then
		for i = 0, addressSize - 1 do
			offset = offset | (saveFileBuffer[indirectAddress + i + 1] << i * 8)
		end
	end
		
	for i = 0, valueSize - 1 do
		value = value | (saveFileBuffer[offset + address + i + 1] << i * 8)
	end
	
	return value
end

function setValueInSaveFile(value)
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	indirectAddress = tonumber(strArgs[1], 16)
	address = tonumber(strArgs[2], 16)
	addressSize = intArgs[1]
	valueSize = intArgs[2]
	
	offset = 0
	
	if indirectAddress ~= 0 then
		for i = 0, addressSize - 1 do
			offset = offset | (saveFileBuffer[indirectAddress + i + 1] << (i * 8))
		end
	end
		
	for i = 0, valueSize - 1 do
		saveFileBuffer[offset + address + i + 1] = (value & (0xFF << i * 8)) >> (i * 8)
	end
end

function getOffset()
	offset = 0
	for i = 0, 3 do
		offset = offset | (saveFileBuffer[41 + i] << i * 8)
	end
	return offset
end

function calcChecksum(saveDataOffset)
	checksumFileBuffer = {}
	for i = saveDataOffset, saveDataOffset + 393215 do
		checksumFileBuffer[i - saveDataOffset + 1] = saveFileBuffer[i]
	end
	return md5.hash(string.char(table.unpack(checksumFileBuffer))) 
end

function setChecksum()
	checksumOffset = getOffset()
	md5hash = calcChecksum(checksumOffset + 17)
	checksum = table.pack(md5hash:byte(1, 16))
	for i = 1, 16 do
		saveFileBuffer[checksumOffset + i] = checksum[i]
	end
end

function getModifiedSaveFile()
	setChecksum()
	return saveFileBuffer
end