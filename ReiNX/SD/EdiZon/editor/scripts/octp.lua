-- octp --

saveFileBuffer = edizon.getSaveFileBuffer()
cachedOffset = {}

function getOffsetKey()
	local strArgs = edizon.getStrArgs()
	local intArgs = edizon.getIntArgs()
	local offsetKey = strArgs[1]
	if strArgs[3] then
		offsetKey = strArgs[3].."$|$|"..(strArgs[1] or 0).."$|$|"..(intArgs[3] or 1)
	end
	return offsetKey
end

function getOffset()
	local strArgs = edizon.getStrArgs()
	local intArgs = edizon.getIntArgs()
	local offsetKey = getOffsetKey()

	local indirectAddress = tonumber(strArgs[1], 16)
	local searchString = strArgs[3]

	if cachedOffset[offsetKey] ~= nil then
		return cachedOffset[offsetKey]
	end
	
	local addressSize = intArgs[1]

	local offset = 0

	if searchString ~= nil and searchString ~= '' then
		local resultNum = intArgs[3] or 1
		local start = 1
		
		if cachedOffset[searchString.."$|$|"..(resultNum-1)] ~= nil then
			start = cachedOffset[searchString.."$|$|"..(resultNum-1)]+2
			resultNum = 1
		else
			start = indirectAddress+1
		end
		
		searchTable = { searchString:byte(1, -1) }
		searchSize = searchString:len()

		local found = 0
		for i = start, #saveFileBuffer do
			if i - 1 + searchSize > #saveFileBuffer then
				break
			end
			for j = 1, searchSize do
				c = saveFileBuffer[i + j -1]
				if c ~= searchTable[j] then
					break
				end
				if j == searchSize then
					found = found + 1
				end
			end
			if found == resultNum then
				offset = i - 1
				break
			end
		end
	elseif indirectAddress ~= 0 then
		for i = 0, addressSize - 1 do
			offset = offset | (saveFileBuffer[indirectAddress + i + 1] << i * 8)
		end
	end

	cachedOffset[offsetKey] = offset
	return offset
end

function getValueFromSaveFile()
	local strArgs = edizon.getStrArgs()
	local intArgs = edizon.getIntArgs()

	local address = tonumber(strArgs[2], 16)
	local valueSize = intArgs[2]

	local offset = getOffset()
	local value = 0

	for i = 0, valueSize - 1 do
		value = value | (saveFileBuffer[offset + address + i + 1] << i * 8)
	end

	return value
end

function setValueInSaveFile(value)
	local strArgs = edizon.getStrArgs()
	local intArgs = edizon.getIntArgs()
	local address = tonumber(strArgs[2], 16)
	local valueSize = intArgs[2]

	local offset = getOffset()

	for i = 0, valueSize - 1 do
		saveFileBuffer[offset + address + i + 1] = (value & (0xFF << i * 8)) >> (i * 8)
	end
end

function getModifiedSaveFile()
	return saveFileBuffer
end
