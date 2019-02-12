-- New Super Mario Bros U Deluxe (modified bin) --
-- by DNA


checksum = require("lib.checksum")

saveFileBuffer = edizon.getSaveFileBuffer()

function getValueFromSaveFile()
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	slot = tonumber(strArgs[1], 16)
	offset = tonumber(strArgs[2], 16)
	addressSize = intArgs[1]
	valueSize = intArgs[2]
	
	value = 0
	
	struct_start = 0x10
	struct_size = 0x207

		
	for i = 0, valueSize - 1 do
		value = value | (saveFileBuffer[struct_start + offset + i + 1 + slot * struct_size] << i * 8)
	end
	
	
	return value
end

function StarCoins()
	strArgs = edizon.getStrArgs()
	slot = tonumber(strArgs[1], 16)
	offsetStarCoins = tonumber(strArgs[2], 16)

	for i = 0, 40 do
		saveFileBuffer[i + 1 + offsetStarCoins + 0x10 + 0x207 * slot] = 119
	end
	
	
			
	return "All Star Coins unlocked"
end


function setValueInSaveFile(value)
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	slot = tonumber(strArgs[1], 16)
	offset = tonumber(strArgs[2], 16)
	dummy = strArgs[3]
	
	addressSize = intArgs[1]
	valueSize = intArgs[2]
	
	struct_start = 0x10
	struct_size = 0x207
	

	if dummy == "Lives" then
		for i = 0, 4 do
			saveFileBuffer[struct_start + offset + i + 1 + slot * struct_size] = value
		end
	else
		for i = 0, valueSize - 1 do
			saveFileBuffer[struct_start + offset + i + 1 + slot * struct_size] = (value & (0xFF << i * 8)) >> (i * 8)
		end
	end
	
end

function copyquickslot()
    s1=0x10
	s2=0x218
	s3=0x420
	qs1=0xC40
	qs2=0xE48
	qs3=0x1050
	
	if saveFileBuffer[qs1+1] > 0 then
		for i = 0, struct_size - 3 do
			saveFileBuffer[i + s1 + 1] = saveFileBuffer[i + qs1 + 1]
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
	copyquickslot()
	setChecksum()
	return saveFileBuffer
end