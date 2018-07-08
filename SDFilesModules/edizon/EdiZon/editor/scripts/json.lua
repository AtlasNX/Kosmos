-- json --

json = require("lib.json")

function convertToString(t)
	str = ""
	for i, arg in ipairs(t) do
		str = str..string.char(arg)
	end
	
	return	str
end

function convertToTable(s)
	t = {}
	
	for i = 1, #s do
		t[i] = string.byte(s:sub(i, i))
	end
	
	return t
end

saveFileBuffer = json.decode(convertToString(edizon.getSaveFileBuffer()))

function getValueFromSaveFile()
	strArgs = edizon.getStrArgs()
	
	item = saveFileBuffer
	
	for i, tag in pairs(strArgs) do
		if type(item) ~= "table" then break end
	
		item = item[tag]
	end
	
	return item
end

function setValueInSaveFile(value)
	local items = saveFileBuffer
	strArgs = edizon.getStrArgs()
	
	local ref = items
	
	for i, arg in ipairs(strArgs) do
		if i == #strArgs then
			ref[arg] = value
		else 
			ref = ref[arg]
		end
	end
end

function getModifiedSaveFile()
	encoded = json.encode(saveFileBuffer)
	convertedTable = {}
	convertedTable = convertToTable(encoded)
		
	print(convertedTable[#convertedTable - 1])
		
	return convertedTable
end
