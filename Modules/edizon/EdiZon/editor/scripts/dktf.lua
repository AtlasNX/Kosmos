-- Donkey Kong Tropical Freeze (modified bin) --
-- IMPORTANT: 
--   Save file is big-endian!
--   getValueFromSaveFile & setValueInSaveFile have been modified accordingly
--   Save file is always 0x40000 bytes in size, where the first 4 bytes are
--   the CRC32 of [0x04 .. 0x40000]

checksum = require("lib.checksum")

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
			offset = offset | (saveFileBuffer[indirectAddress + i + 1] << ((addressSize - i - 1) * 8))
		end
	end
		
	for i = 0, valueSize - 1 do
		value = value | (saveFileBuffer[offset + address + i + 1] << ((valueSize - i - 1) * 8))
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
			offset = offset | (saveFileBuffer[indirectAddress + i + 1] << ((addressSize - i - 1) * 8))
		end
	end
		
	for i = 0, valueSize - 1 do
		saveFileBuffer[offset + address + i + 1] = (value & (0xFF << (valueSize - i - 1) * 8)) >> ((valueSize - i - 1) * 8)
	end
end

function setChecksum()
	gameFileBuffer = {}
	for i = 1, 262140 do
		gameFileBuffer[i] = saveFileBuffer[i + 4]
	end
	crc = checksum.crc32(string.char(table.unpack(gameFileBuffer)))
	crc = crc ~ 0xFFFFFFFF
   for i = 0,3 do                                           
      saveFileBuffer[i + 1] = (crc & (0xFF000000 >> (i * 8))) >> (24 - i * 8)
   end
end

function getModifiedSaveFile()
	setChecksum()
	return saveFileBuffer
end

