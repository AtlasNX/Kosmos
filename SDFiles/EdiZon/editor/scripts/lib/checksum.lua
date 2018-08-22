-- Copyright (c) 2015  Phil Leblanc  -- see LICENSE file
------------------------------------------------------------
-- checksums:  adler-32, crc-32


------------------------------------------------------------
local byte = string.byte

------------------------------------------------------------
-- adler32

local function adler32(s)
	-- return adler32 checksum  (uint32)
	-- adler32 is a checksum defined by Mark Adler for zlib
	-- (based on the Fletcher checksum used in ITU X.224)
	-- implementation based on RFC 1950 (zlib format spec), 1996
	local prime = 65521 --largest prime smaller than 2^16
	local s1, s2 = 1, 0
	-- limit s size to ensure that modulo prime can be done only at end
	-- 2^40 bytes should be enough for pure Lua with 64-bit integers...
	if #s > (1 << 40) then error("adler32: string too large") end
	for i = 1,#s do
		local b = byte(s, i)
		s1 = s1 + b
		s2 = s2 + s1
		-- no need to test or compute mod prime every turn.
	end
	s1 = s1 % prime
	s2 = s2 % prime
	return (s2 << 16) + s1
end --adler32()

local function crc32_nt(s)
	-- return crc32 checksum of string s as an integer
	-- uses no lookup table
	-- inspired by crc32b at
	-- http://www.hackersdelight.org/hdcodetxt/crc.c.txt
	local b, crc, mask
	crc = 0xffffffff
	for i = 1, #s do
		b = byte(s, i)
		crc = crc ~ b
		for _ = 1, 8 do --eight times
			mask = -(crc & 1)
			crc = (crc >> 1) ~ (0xedb88320 & mask)
		end
	end--for
	return (~crc) & 0xffffffff
end --crc32_nt()

local function crc32(s, lt)
	-- return crc32 checksum of string as an integer
	-- use lookup table lt if provided or create one on the fly
	-- if lt is empty, it is initialized.
	lt = lt or {}
	local b, crc, mask
	if not lt[1] then -- setup table
		for i = 1, 256 do
			crc = i - 1
			for _ = 1, 8 do --eight times
				mask = -(crc & 1)
				crc = (crc >> 1) ~ (0xedb88320 & mask)
			end
			lt[i] = crc
		end--for
	end--if
	-- compute the crc
	crc = 0xffffffff
	for i = 1, #s do
		b = byte(s, i)
		crc = (crc >> 8) ~ lt[((crc ~ b) & 0xFF) + 1]
	end
	return (~crc) & 0xffffffff
end --crc32()


return  {
	-- module
	adler32 = adler32,
	crc32_nt = crc32_nt,
	crc32 = crc32,
	}
