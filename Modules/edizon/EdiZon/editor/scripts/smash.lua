-- smash --

saveFileBuffer = edizon.getSaveFileBuffer()

function getValueFromSaveFile()
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	startingAddress = tonumber(strArgs[1], 16)
	endingAddress= tonumber(strArgs[2], 16)
	addressSize = intArgs[1]
	valueSize = intArgs[2]
	
	value = 0

	if startingAddress ~= 0 then
		if strArgs[1] == "53FF60" then
			value = 1
			for i = 0, endingAddress - startingAddress - 1, 16 do
				value = value & saveFileBuffer[startingAddress+i+1] & saveFileBuffer[startingAddress+i+1+4] & saveFileBuffer[startingAddress+i+1+8]
			end
		end

		if strArgs[1] == "426C77" then
			value = 255
			for i = 0, endingAddress - startingAddress - 1,28 do
				value = value & saveFileBuffer[startingAddress+i+1]
			end
		end

		if strArgs[1] == "426C76" then
			for k=0, endingAddress - startingAddress - 1, 28 do
				for i = 0, valueSize - 1 do
					value = value | (saveFileBuffer[(startingAddress+k) + i + 1] << i * 8)
				end
				value = value & 65293
			end
		end


		if strArgs[1] == "555BB8" then
			value = 255
			for i = 0, endingAddress- startingAddress - 1 do
				value = value & saveFileBuffer[startingAddress+i+1]
			end
		end

		if strArgs[1] == "00D0" then
			value = 255
			for i = 0, endingAddress - startingAddress - 1 do
				value = value & saveFileBuffer[startingAddress+i+1]
			end
		end

		if strArgs[1] == "434750" then
			for k=0, endingAddress- startingAddress - 1, 52 do
				for i = 0, valueSize - 1 do
					value = value | (saveFileBuffer[(startingAddress+k) + i + 17] << i * 8)
				end
				value = value & 65294
			end
		end

	else
		for i = 0, valueSize - 1 do
			value = value | (saveFileBuffer[endingAddress+ i + 1] << i * 8)
		end
	end
	
	return value
end

function setValueInSaveFile(value)
	strArgs = edizon.getStrArgs()
	intArgs = edizon.getIntArgs()
	startingAddress = tonumber(strArgs[1], 16)
	endingAddress = tonumber(strArgs[2], 16)
	addressSize = intArgs[1]
	valueSize = intArgs[2]
	
	if startingAddress ~= 0 then
		if strArgs[1] == "53FF60" then
			for i = 0, endingAddress - startingAddress - 1, 16 do
				saveFileBuffer[startingAddress+i+1] = value
				saveFileBuffer[startingAddress+i+1+4] = value
				saveFileBuffer[startingAddress+i+1+8] = value
			end
		end

		if strArgs[1] == "426C77" then
			for i = 0, endingAddress - startingAddress - 1,28 do
				saveFileBuffer[startingAddress + i + 1] = value
			end
		end

		if strArgs[1] == "426C76" then
			for i = 0, endingAddress - startingAddress - 1,28 do
				for m = 0, valueSize - 1 do
					saveFileBuffer[startingAddress + i + m + 1] = (value & (0xFF << m * 8)) >> (m * 8)
				end
			end
		end

		if strArgs[1] == "555BB8" then
			for i = 0, endingAddress - startingAddress - 1 do
				saveFileBuffer[startingAddress + i + 1] = value
			end
		end

		if strArgs[1] == "00D0" then
			for i = 0, endingAddress - startingAddress - 1 do
				saveFileBuffer[startingAddress + i + 1] = value
			end
		end

		if strArgs[1] == "434750" then
			if value == 0 then
				for i = 0, endingAddress - startingAddress - 1,52 do
					for m = 0, valueSize - 1 do
						saveFileBuffer[startingAddress + i + m + 1] = (4294967295 & (0xFF << m * 8)) >> (m * 8)
					end

					for m = 0, valueSize - 1 do
						saveFileBuffer[startingAddress + i + m + 11] = (65535 & (0xFF << m * 8)) >> (m * 8)
					end

					for m = 0, valueSize - 1 do
						saveFileBuffer[startingAddress + i + m + 17] = (14 & (0xFF << m * 8)) >> (m * 8)
					end
				end
			else
				spirit=1
				for i = 0, endingAddress - startingAddress - 1,52 do
					for m = 0, valueSize - 1 do
						saveFileBuffer[startingAddress + i + m + 1] = (spirit & (0xFF << m * 8)) >> (m * 8)
					end

					for m = 0, valueSize - 1 do
						saveFileBuffer[startingAddress + i + m + 11] = (spirit-1 & (0xFF << m * 8)) >> (m * 8)
					end

					for m = 0, valueSize - 1 do
						saveFileBuffer[startingAddress + i + m + 17] = (value & (0xFF << m * 8)) >> (m * 8)
					end
					spirit=spirit+1
				end
			end				
		end

	else	
		for i = 0, valueSize - 1 do
			saveFileBuffer[endingAddress+ i + 1] = (value & (0xFF << i * 8)) >> (i * 8)
		end
	end
end

function getModifiedSaveFile()
	return saveFileBuffer
end