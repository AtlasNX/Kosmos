#!/usr/bin/python
import os, json, zipfile, time, datetime, hashlib

print("Content-type: text/html\n\n")

def zipdir(path, ziph):
    for root, dirs, files in os.walk(path):        
        for file in files:
            if root == "." and (file == "icon.png"):
                continue
            ziph.write(os.path.join(root, file))

try:
    os.mkdir("zips")
except:
    pass

curdir = os.getcwd()
packages = {}
packages["packages"] = []

for package in os.listdir("packages"):
    if os.path.isfile("packages/" + package):
        continue
        
    zipf = zipfile.ZipFile("zips/" + package + ".zip", 'w', zipfile.ZIP_DEFLATED)
    os.chdir(curdir + "/packages/" + package)
    
    # generate a manifest to go inside of this zip
    # TODO: pull in any existing manifest and only write U entries
    # omitted files
    manifest = ""
    for root, dirs, files in os.walk("."):
        for file in files:
            if file == "manifest.install" or file == "icon.png" or file == "info.json" or file == "screen.png":
                continue
            manifest += "U: %s\n" % os.path.join(root, file)[2:]
    manifest_file = open("manifest.install", "w")
    manifest_file.write(manifest)
    manifest_file.close()
    
    print("Zipping %s...<br>" % package)
    zipdir(".", zipf)
    zipf.close()

    # Detail zip package size in KB's
    filesize = os.path.getsize(curdir + "/zips/" + package + ".zip")/1024

    # Date last updated (assumption is that if the app is updated the info.json would be)
    updated = time.strftime('%d/%m/%Y', time.gmtime(os.path.getmtime(curdir + "/packages/" + package + "/info.json")))

    #md5 of package zip
    filehash = hashlib.md5()
    filehash.update(open(curdir + "/zips/" + package + ".zip").read())
    mdhex = filehash.hexdigest()

    # this line isn't confusing at all (additonal info makes it less so)
    packages["packages"].append({"name": package, "filesize": filesize, "updated": updated, "md5": mdhex})
    
    # if a info.json file exists, load properties from it
    if os.path.exists("info.json"):
        target = packages["packages"][-1]
        props = json.load(open("info.json", "r"))
        vals = ["title", "author", "category", "version", "description", "details", "url", "license"]
        for val in vals:
            if val in props:
                target[val] = props[val]
    
    os.chdir(curdir)
    
out = open("repo.json", "w")
json.dump(packages, out, indent=4)
out.close()

print("All Done!<br>")
