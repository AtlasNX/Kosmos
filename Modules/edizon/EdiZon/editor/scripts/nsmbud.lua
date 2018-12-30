-- New Super Mario Bros U Deluxe (modified bin) --
-- by DNA


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
	dummy = strArgs[3]
	
	addressSize = intArgs[1]
	valueSize = intArgs[2]
	
	offset = 0
	struct_size = 0x207
	s1=0x10
	s2=0x218
	s3=0x420
	qs1=0xC40
	qs2=0xE48
	qs3=0x1050
	
	if indirectAddress ~= 0 then
		for i = 0, addressSize - 1 do
			offset = offset | (saveFileBuffer[indirectAddress + i + 1] << (i * 8))
		end
	end
	
	
	if saveFileBuffer[qs1+1] > 0 then
	
		--print(saveFileBuffer[qs1+1])
	
		for i = 0, struct_size - 3 do

			saveFileBuffer[i + s1 + 1] = saveFileBuffer[i + qs1 + 1]
			--print(s1 + i .. "-" .. qs1 + i)
			--print(saveFileBuffer[i + s1 + 1] .. "-" .. saveFileBuffer[i + qs1 + 1] .. "\n")
			saveFileBuffer[i + qs1 + 1] = 0
		end
		
		saveFileBuffer[qs1 + struct_size - 2] = 0xAC
		saveFileBuffer[qs1 + struct_size - 1] = 0x72
		saveFileBuffer[qs1 + struct_size] = 0x7B
		saveFileBuffer[qs1 + struct_size + 1] = 0x17
	end
	
	if saveFileBuffer[qs2+1] > 0 then
		for i = 0, struct_size - 3 do
			saveFileBuffer[i + s2 + 1] = saveFileBuffer[i + qs2 + 1]
			saveFileBuffer[i + qs2 + 1] = 0
		end
		
		saveFileBuffer[qs2 + struct_size - 2] = 0xAC
		saveFileBuffer[qs2 + struct_size - 1] = 0x72
		saveFileBuffer[qs2 + struct_size] = 0x7B
		saveFileBuffer[qs2 + struct_size + 1] = 0x17
	end
	
	if saveFileBuffer[qs3+1] > 0 then
		for i = 0, struct_size - 3 do
			saveFileBuffer[i + s3 + 1] = saveFileBuffer[i + qs3 + 1]
			saveFileBuffer[i + qs3 + 1] = 0
		end
		
		saveFileBuffer[qs3 + struct_size - 2] = 0xAC
		saveFileBuffer[qs3 + struct_size - 1] = 0x72
		saveFileBuffer[qs3 + struct_size] = 0x7B
		saveFileBuffer[qs3 + struct_size + 1] = 0x17
	end

		if dummy == "Lives" then
		for i = 0, 4 do
			saveFileBuffer[offset + address + i + 1] = value
		end
	else
		for i = 0, valueSize - 1 do
			saveFileBuffer[offset + address + i + 1] = (value & (0xFF << i * 8)) >> (i * 8)
		end
	end
	
end


function setChecksum()
	gameFileBuffer = {}
	
	struct_start = 0x10
	struct_size = 0x207
	
	
	for num_struct = 0,2 do 
	address = 1
	
		for i = 1 + struct_start + num_struct * struct_size + num_struct, 1 + struct_start + struct_size * (num_struct + 1) - 4 + num_struct do
			gameFileBuffer[address] = saveFileBuffer[i]
			address = address + 1
		end
		
			crc = checksum.crc32(string.char(table.unpack(gameFileBuffer)))
   
		for i = 0, 3 do                                           
			saveFileBuffer[i + 1 + struct_start + struct_size * (num_struct + 1) - 3 + num_struct] = (crc & (0xFF000000 >> (i * 8))) >> (24 - i * 8)
		end
	end 
	
end

function getModifiedSaveFile()
	setChecksum()
	return saveFileBuffer
end