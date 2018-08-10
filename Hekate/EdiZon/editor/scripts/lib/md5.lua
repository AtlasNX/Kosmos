-- Copyright (c) 2017  Phil Leblanc  -- see LICENSE file

------------------------------------------------------------------------
-- md5 hash - see RFC 1321 - https://www.ietf.org/rfc/rfc1321.txt

local spack, sunpack = string.pack, string.unpack

------------------------------------------------------------------------

local function FF(a, b, c, d, x, s, ac)
	a = (a + ((b & c) | ((~b) & d)) + x + ac) & 0xffffffff
	a = ((a << s) | (a >> (32-s))) & 0xffffffff
	a = (a + b) & 0xffffffff
	return a
end

local function GG(a, b, c, d, x, s, ac)
	a = (a + ((b & d) | c & (~d) ) + x + ac) & 0xffffffff
	a = ((a << s) | (a >> (32-s))) & 0xffffffff
	a = (a + b) & 0xffffffff
	return a
end

local function HH(a, b, c, d, x, s, ac)
	a = (a + ((b ~ c ~ d)) + x + ac) & 0xffffffff
	a = ((a << s) | (a >> (32-s))) & 0xffffffff
	a = (a + b) & 0xffffffff
	return a
end

local function II(a, b, c, d, x, s, ac)
	a = (a + (c ~ (b | ~d)) + x + ac) & 0xffffffff
	a = ((a << s) | (a >> (32-s))) & 0xffffffff
	a = (a + b) & 0xffffffff
	return a
end

local function transform(state, input, i, t)
	-- process the 64-byte input block in string 'input' at offset 'i'
	-- t is a uint32[16] array. It is passed as a parameter
	-- for performance reasons
	--
	local a, b, c, d = state[1], state[2], state[3], state[4]

	-- load array
	for j = 1, 16 do
		t[j] = sunpack("<I4", input, i)
		i = i + 4
	end

    -- Round 1
    a = FF (a, b, c, d, t[ 1], 7, 0xd76aa478)
    d = FF (d, a, b, c, t[ 2], 12, 0xe8c7b756)
    c = FF (c, d, a, b, t[ 3], 17, 0x242070db)
    b = FF (b, c, d, a, t[ 4], 22, 0xc1bdceee)
    a = FF (a, b, c, d, t[ 5], 7, 0xf57c0faf)
    d = FF (d, a, b, c, t[ 6], 12, 0x4787c62a)
    c = FF (c, d, a, b, t[ 7], 17, 0xa8304613)
    b = FF (b, c, d, a, t[ 8], 22, 0xfd469501)
    a = FF (a, b, c, d, t[ 9], 7, 0x698098d8)
    d = FF (d, a, b, c, t[10], 12, 0x8b44f7af)
    c = FF (c, d, a, b, t[11], 17, 0xffff5bb1)
    b = FF (b, c, d, a, t[12], 22, 0x895cd7be)
    a = FF (a, b, c, d, t[13], 7, 0x6b901122)
    d = FF (d, a, b, c, t[14], 12, 0xfd987193)
    c = FF (c, d, a, b, t[15], 17, 0xa679438e)
    b = FF (b, c, d, a, t[16], 22, 0x49b40821)

    -- Round 2
    a = GG (a, b, c, d, t[ 2], 5, 0xf61e2562)
    d = GG (d, a, b, c, t[ 7], 9, 0xc040b340)
    c = GG (c, d, a, b, t[12], 14, 0x265e5a51)
    b = GG (b, c, d, a, t[ 1], 20, 0xe9b6c7aa)
    a = GG (a, b, c, d, t[ 6], 5, 0xd62f105d)
    d = GG (d, a, b, c, t[11], 9,  0x2441453)
    c = GG (c, d, a, b, t[16], 14, 0xd8a1e681)
    b = GG (b, c, d, a, t[ 5], 20, 0xe7d3fbc8)
    a = GG (a, b, c, d, t[10], 5, 0x21e1cde6)
    d = GG (d, a, b, c, t[15], 9, 0xc33707d6)
    c = GG (c, d, a, b, t[ 4], 14, 0xf4d50d87)
    b = GG (b, c, d, a, t[ 9], 20, 0x455a14ed)
    a = GG (a, b, c, d, t[14], 5, 0xa9e3e905)
    d = GG (d, a, b, c, t[ 3], 9, 0xfcefa3f8)
    c = GG (c, d, a, b, t[ 8], 14, 0x676f02d9)
    b = GG (b, c, d, a, t[13], 20, 0x8d2a4c8a)

    -- Round 3
    a = HH (a, b, c, d, t[ 6], 4, 0xfffa3942)
    d = HH (d, a, b, c, t[ 9], 11, 0x8771f681)
    c = HH (c, d, a, b, t[12], 16, 0x6d9d6122)
    b = HH (b, c, d, a, t[15], 23, 0xfde5380c)
    a = HH (a, b, c, d, t[ 2], 4, 0xa4beea44)
    d = HH (d, a, b, c, t[ 5], 11, 0x4bdecfa9)
    c = HH (c, d, a, b, t[ 8], 16, 0xf6bb4b60)
    b = HH (b, c, d, a, t[11], 23, 0xbebfbc70)
    a = HH (a, b, c, d, t[14], 4, 0x289b7ec6)
    d = HH (d, a, b, c, t[ 1], 11, 0xeaa127fa)
    c = HH (c, d, a, b, t[ 4], 16, 0xd4ef3085)
    b = HH (b, c, d, a, t[ 7], 23,  0x4881d05)
    a = HH (a, b, c, d, t[10], 4, 0xd9d4d039)
    d = HH (d, a, b, c, t[13], 11, 0xe6db99e5)
    c = HH (c, d, a, b, t[16], 16, 0x1fa27cf8)
    b = HH (b, c, d, a, t[ 3], 23, 0xc4ac5665)

    -- Round 4
    a = II (a, b, c, d, t[ 1], 6, 0xf4292244)
    d = II (d, a, b, c, t[ 8], 10, 0x432aff97)
    c = II (c, d, a, b, t[15], 15, 0xab9423a7)
    b = II (b, c, d, a, t[ 6], 21, 0xfc93a039)
    a = II (a, b, c, d, t[13], 6, 0x655b59c3)
    d = II (d, a, b, c, t[ 4], 10, 0x8f0ccc92)
    c = II (c, d, a, b, t[11], 15, 0xffeff47d)
    b = II (b, c, d, a, t[ 2], 21, 0x85845dd1)
    a = II (a, b, c, d, t[ 9], 6, 0x6fa87e4f)
    d = II (d, a, b, c, t[16], 10, 0xfe2ce6e0)
    c = II (c, d, a, b, t[ 7], 15, 0xa3014314)
    b = II (b, c, d, a, t[14], 21, 0x4e0811a1)
    a = II (a, b, c, d, t[ 5], 6, 0xf7537e82)
    d = II (d, a, b, c, t[12], 10, 0xbd3af235)
    c = II (c, d, a, b, t[ 3], 15, 0x2ad7d2bb)
    b = II (b, c, d, a, t[10], 21, 0xeb86d391)

	state[1] = (state[1] + a) & 0xffffffff
	state[2] = (state[2] + b) & 0xffffffff
	state[3] = (state[3] + c) & 0xffffffff
	state[4] = (state[4] + d) & 0xffffffff
end --transform()

local function md5(input)
	-- initialize state
	local state = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476 }
	local inputlen = #input
	local inputbits = inputlen * 8 -- input length in bits
	local r = inputlen -- number of unprocessed bytes
	local i = 1 -- index in input string
	local ibt = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} -- input block uint32[16]
	-- process as many 64-byte blocks as possible
	while r >= 64 do
		-- process block
		transform(state, input, i, ibt)
		i = i + 64 -- update input index
		r = r - 64 -- update number of unprocessed bytes
	end
	-- finalize.  must append to input a mandatory 0x80 byte, some
	--  padding, and the input bit-length ('inputbits')
	local lastblock -- the rest of input .. some padding .. inputbits
	local padlen -- padding length in bytes
	if r < 56 then padlen = 55 - r else padlen = 119 - r end
	lastblock = input:sub(i) -- remaining input
		.. '\x80' .. ('\0'):rep(padlen)  --padding
		.. spack("<I8", inputbits) -- length in bits
	assert(#lastblock == 64 or #lastblock == 128)
	transform(state, lastblock, 1, ibt)
	if #lastblock == 128 then
		transform(state, lastblock, 65, ibt)
	end
	-- return the digest
	local digest = spack("<I4I4I4I4", state[1], state[2], state[3], state[4])
	return digest
end --md5()

--~ bin = require "plc.bin"
--~ print(bin.stohex(md5'abc'))
--~ print(bin.stohex(md5""))

return {  -- md5 module
	hash = md5,
}
