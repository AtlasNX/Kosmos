Packages should be inserted into this file on the server, and then the `repogen.py` should be executed to generate zips of the packages.

## Directory Structure
Any folders in this directory are valid package names. For instance, if there is a folder named "space" here, anything inside of that folder will be zipped into the package that will be available under the name "space".

There can also be a manifest.install file specified, as well as an icon.png and an info.json file, containing metadata about the package. All of those three files are optional.
