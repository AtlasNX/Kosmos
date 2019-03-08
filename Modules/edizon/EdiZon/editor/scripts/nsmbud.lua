-- New Super Mario Bros U Deluxe (modified bin) --
-- by DNA


checksum = require("lib.checksum")

struct_start = 0x10
quick_struct_start = 0xC40
struct_size = 0x208
offset_pipes = 0x21
offset_no_crash = 0x51
offset_unlock_nabbit = 0x4D
saveFileBuffer = edizon.getSaveFileBuffer()

	
	for slot_num = 0, 5 do
		if saveFileBuffer[quick_struct_start + 1 + slot_num * struct_size] > 0 then
			for i = 0, struct_size do
				saveFileBuffer[i + struct_start + 1 + slot_num * struct_size] = saveFileBuffer[i + quick_struct_start + 1 + slot_num * struct_size]
				saveFileBuffer[i + quick_struct_start + 1 + slot_num * struct_size] = 0
			end
		
			saveFileBuffer[quick_struct_start + struct_size - 3 + slot_num * struct_size] = 0xAC
			saveFileBuffer[quick_struct_start + struct_size - 2 + slot_num * struct_size] = 0x72
			saveFileBuffer[quick_struct_start + struct_size - 1 + slot_num * struct_size] = 0x7B
			saveFileBuffer[quick_struct_start + struct_size + slot_num * struct_size] = 0x17
		end
	end
	
	


function getValueFromSaveFile()
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	slot = tonumber(strArgs[1], 16)
	offset = tonumber(strArgs[2], 16)
	addressSize = intArgs[1]
	valueSize = intArgs[2]
	
	value = 0
	

	
		
	for i = 0, valueSize - 1 do
		value = value | (saveFileBuffer[struct_start + offset + i + 1 + slot * struct_size] << i * 8)
	end
	
	
	return value
end

function StarCoins()
	strArgs = edizon.getStrArgs()
	slot = tonumber(strArgs[1], 16)
	offset = tonumber(strArgs[2], 16)

	for i = 0, 40 do
		saveFileBuffer[i + 1 + offset + struct_start + struct_size * slot] = 119
	end
	
	
			
	return "Star Coins unlocked"
end

function UnlockLevels()
	strArgs = edizon.getStrArgs()
	slot = tonumber(strArgs[1], 16)
	offset = tonumber(strArgs[2], 16)
	
	lvl_array = {0x47, 0xCF, 0x47, 0x47, 0x47, 0x1, 0x1, 0x1, 0, 0, 0, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0xCF, 0x47, 0x47, 0x1, 0x1, 0x1, 0x42, 0, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x1, 0x1, 0x1, 0, 0, 0x47, 0xCF, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x1, 0x1, 0x1, 0, 0, 0, 0x42, 0x47, 0xCF, 0x47, 0x47, 0x47, 0xCF, 0x47, 0xCF, 0xCF, 0x47, 0x47, 0x1, 0x1, 0x1, 0, 0x42, 0, 0x47, 0xCF, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0xCF, 0x47, 0x47, 0x1, 0x1, 0x1, 0xCF, 0x47, 0xCF, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x1, 0x1, 0x1, 0, 0, 0xCF, 0x47, 0x47, 0x47, 0xCF, 0x47, 0x47, 0x47, 0x47, 0x47, 0x47, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1}
	
	for i = 1, #lvl_array do
		if (saveFileBuffer[i + offset + struct_start + struct_size * slot] < 0x42) or (lvl_array[i] == 0xCF and saveFileBuffer[i + offset + struct_start + struct_size * slot] == 1) then 
			saveFileBuffer[i + offset + struct_start + struct_size * slot] = lvl_array[i]
		elseif (saveFileBuffer[i + offset + struct_start + struct_size * slot] == 0x43) and (lvl_array[i] == 0xCF) then
			saveFileBuffer[i + offset + struct_start + struct_size * slot] = 0xC3
		end
	end

	
	if slot >=3 then
		saveFileBuffer[1 + struct_start + struct_size * slot + offset_pipes - 1] = 0xFF
	else
		saveFileBuffer[1 + struct_start + struct_size * slot + offset_pipes] = 0xFF
	end
	
	
	if saveFileBuffer[1 + struct_start + struct_size * slot + offset_no_crash] < 3 then
		saveFileBuffer[1 + struct_start + struct_size * slot + offset_no_crash] = 0x03
	end
	
	if saveFileBuffer[1 + struct_start + struct_size * slot + offset_unlock_nabbit] == 0xFF then
		saveFileBuffer[1 + struct_start + struct_size * slot + offset_unlock_nabbit] = 0x00
		saveFileBuffer[1 + struct_start + struct_size * slot + offset_unlock_nabbit + 1] = 0x00
	end
			
	return "Levels unlocked"
end

function setValueInSaveFile(value)
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	slot = tonumber(strArgs[1], 16)
	offset = tonumber(strArgs[2], 16)
	dummy = strArgs[3]
	
	addressSize = intArgs[1]
	valueSize = intArgs[2]
	
	

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
	
	for slot_num = 0, 5 do
		if saveFileBuffer[qs1 + 1 + slot_num * struct_size] > 0 then
			for i = 0, struct_size do
				saveFileBuffer[i + s1 + 1 + slot_num * struct_size] = saveFileBuffer[i + qs1 + 1 + slot_num * struct_size]
				saveFileBuffer[i + qs1 + 1 + slot_num * struct_size] = 0
			end
		
			saveFileBuffer[qs1 + struct_size - 3 + slot_num * struct_size] = 0xAC
			saveFileBuffer[qs1 + struct_size - 2 + slot_num * struct_size] = 0x72
			saveFileBuffer[qs1 + struct_size - 1 + slot_num * struct_size] = 0x7B
			saveFileBuffer[qs1 + struct_size + slot_num * struct_size] = 0x17
		end
	end
	

end


function setChecksum()
	gameFileBuffer = {}
	
	
	
	for num_struct = 0,5 do 
	address = 1
	
		for i = 1 + struct_start + num_struct * struct_size,  struct_start + (struct_size * (num_struct + 1)) - 4 do
			gameFileBuffer[address] = saveFileBuffer[i]			
			address = address + 1
		end
		
			crc = checksum.crc32(string.char(table.unpack(gameFileBuffer)))
   
		for i = 0, 3 do                                           
			saveFileBuffer[i + struct_start + struct_size * (num_struct + 1) - 3 ] = (crc & (0xFF000000 >> (i * 8))) >> (24 - i * 8)
		end
	end 
	
end

function getModifiedSaveFile()
	--copyquickslot()
	setChecksum()
	return saveFileBuffer
end