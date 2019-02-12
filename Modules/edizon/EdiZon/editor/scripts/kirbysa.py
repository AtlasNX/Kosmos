# kirbysa.py - by Ac_K

import edizon

saveFileBuffer = edizon.getSaveFileBuffer()
save_slot_id   = 0
save_slots     = [saveFileBuffer[0x100000:0x200000], saveFileBuffer[0x200000:0x300000], saveFileBuffer[0x300000:0x400000]]

def find_offset(section_name, item_name):
    section_offset = save_slots[save_slot_id].index(section_name.encode())

    section_buffer = save_slots[save_slot_id][section_offset:section_offset + 0x1000]

    item_offset = section_buffer.index(item_name.encode())

    return section_offset + item_offset

def check_slot_exist(index):
    if index == 0:
        meta_buffer = saveFileBuffer[0x100002:0x100006].decode()
    elif index == 1:
        meta_buffer = saveFileBuffer[0x200002:0x200006].decode()
    elif index == 2:
        meta_buffer = saveFileBuffer[0x300002:0x300006].decode()
    else:
        meta_buffer = ""

    if meta_buffer == "meta":
        return True
    else:
        return False

def getDummyValue():
    return save_slot_id + 1

def setDummyValue(value):
    global save_slot_id

    value -= 1

    if check_slot_exist(value):
        save_slot_id = value
    else:
        save_slot_id = 0

def getValueFromSaveFile():
    strArgs = edizon.getStrArgs()
    intArgs = edizon.getIntArgs()

    sectionId = strArgs[0]
    itemId    = strArgs[1]
    padding   = int(strArgs[2], 16)

    valueSize = intArgs[0]

    item_offset = find_offset(sectionId, itemId) + padding

    value = 0

    for i in range(0, valueSize):
        value = value | (save_slots[save_slot_id][item_offset + i] << i * 8)

    return value

def setValueInSaveFile(value):
    global save_slots

    strArgs = edizon.getStrArgs()
    intArgs = edizon.getIntArgs()

    sectionId = strArgs[0]
    itemId    = strArgs[1]
    padding   = int(strArgs[2], 16)

    valueSize = intArgs[0]

    item_offset = find_offset(sectionId, itemId) + padding

    for i in range(0, valueSize):
        save_slots[save_slot_id][item_offset + i] = (value & (0xFF << i * 8)) >> (i * 8)

def getModifiedSaveFile():
    new_save_buffer = saveFileBuffer[0:0x100000] + save_slots[0] + save_slots[1] + save_slots[2]

    return new_save_buffer