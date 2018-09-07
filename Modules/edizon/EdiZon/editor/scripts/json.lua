-- json --

json = require("lib.json")


saveFileString = edizon.getSaveFileString()
saveFileString = saveFileString:gsub('{%s*}', '{"edizon":true}')
saveFileBuffer = json.decode(saveFileString)

function getValueFromSaveFile()
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()

	item = saveFileBuffer
	
	for i, tag in pairs(strArgs) do
		if type(item) ~= "table" then break end
	
		if string.sub(tag, 1, 1) == "\\" then
			tag = tonumber(tag:sub(2)) + 1
			
			if tag == nil then return 0 end
		end
	
		item = item[tag]
	end
		
	if intArgs[1] == 0 then
		return item
	else
		return item and 1 or 0
	end
end

function setValueInSaveFile(value)
	local items = saveFileBuffer
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	
	local ref = items
		
	for i, tag in ipairs(strArgs) do
		
		if string.sub(tag, 1, 1) == "\\" then
			tag = tonumber(tag:sub(2)) + 1
		end
		if i == #strArgs then
			if intArgs[1] == 0 then
				ref[tag] = value
			else
				ref[tag] = (value == 1)
			end
		else 
			ref = ref[tag]
		end
		
	end
end

local function convertToTable(s)
	t = {}
	
	for i = 1, #s do
		t[i] = string.byte(s:sub(i, i))
	end
	
	return t
end

function getModifiedSaveFile()
	encoded = json.encode(saveFileBuffer)
	
	encoded = encoded:gsub('{"edizon":true}', '{}')
	
	convertedTable = {}
	convertedTable = convertToTable(encoded)
				
	return convertedTable
end
