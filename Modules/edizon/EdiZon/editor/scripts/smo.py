# SMO.py - by Ac_K
# Thanks to WerWolv and Leoetlino for helped me!

import edizon, io, operator

from byml import *

savedata_buffer = edizon.getSaveFileBuffer()
savedata_header = savedata_buffer[0:0xC]
savedata_dict   = bymlbuffer_to_object(savedata_buffer[0x10:]) # skip the save data header

# TODO: Add more editable values :P
# print(savedata_dict)

def get_first_empty_index(item_list):
    i = 0

    for x in item_list:
        if x == "":
            return i
        i += 1
    
    return -1

def find_item_index(item_list, item):
    i = 0

    for x in item_list:
        if x == item:
            return i
        i += 1
    
    return -1

def remove_from_category(item_list, item):
    index_item = find_item_index(item_list, item)

    if index_item != -1:
        del item_list[index_item]
        item_list.append('')

def add_to_category(item_list, item):
    if item not in item_list:
        empty_index = get_first_empty_index(item_list)
        item_list[empty_index] = item

def getValueFromSaveFile():
    strArgs = edizon.getStrArgs()
    intArgs = edizon.getIntArgs()

    itemType = strArgs[0]

    if itemType == "Number":
        return int(savedata_dict[strArgs[1]])

    if itemType == "ListItem":
        item_list = savedata_dict[strArgs[1]]
        item_index = find_item_index(item_list, strArgs[2])

        if item_index >= 0:
            return 1
        else:
            return 0

def setValueInSaveFile(value):
    global savedata_dict

    strArgs = edizon.getStrArgs()
    intArgs = edizon.getIntArgs()

    itemType = strArgs[0]

    if itemType == "Number":
        if intArgs[0] == 0:  recast_value = Int(value)
        if intArgs[0] == 4:  recast_value = UInt(value)
        if intArgs[0] == 8:  recast_value = UInt64(value)
        if intArgs[0] == 10: recast_value = Double(value)
        if intArgs[0] == 11: recast_value = Float(value)

        savedata_dict[strArgs[1]] = recast_value

    if itemType == "ListItem":
        item_list = savedata_dict[strArgs[1]]

        if value == 1:
            add_to_category(item_list, strArgs[2])
        if value == 0:
            remove_from_category(item_list, strArgs[2])

        savedata_dict[strArgs[1]] = item_list

def getModifiedSaveFile():
    modified_savedata_byml = object_to_bymlbuffer(savedata_dict, False, 3)
    
    # store new byml size
    byml_size = modified_savedata_byml.getbuffer().nbytes

    # write some dummy bytes to get the right save size
    modified_savedata_byml.seek(0x20000B - 0x10)
    modified_savedata_byml.write(bytes(0x01))
    modified_savedata_byml.seek(0)

    # TODO: Handle the whole 0x10 header
    # 0x00 - u32 - CRC32 checkum from 0x04 to EOF
    # 0x04 - u32 - Version? (Always 0x01)
    # 0x08 - u32 - Save file size
    # 0x0C - u32 - Byml section size

    # write back the header without the byml size
    output_buffer  = savedata_header
    # write the new byml size
    output_buffer += struct.pack("<I", byml_size)
    # write the byml data
    output_buffer += modified_savedata_byml.getvalue() 

    return output_buffer