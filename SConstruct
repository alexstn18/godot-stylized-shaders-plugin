import shutil
import os

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["include/"])

sources = Glob("include/*.cpp")

folder = "demo/addons/GodotStylizedShadersPlugin"

# delete old versions of the libraries (weird way to update your stuff on build but it's worth a try)
for filename in os.listdir(folder):
	file_path = os.path.join(folder, filename)
	if os.path.isfile(file_path):
		os.remove(file_path)

if env["platform"] == "macos":
	file_name = "libGodotStylizedShadersPlugin.{}.{}".format(env["platform"], env["target"])

	library = env.SharedLibrary(
		"{}/{}.framework/{}".format(folder, file_name, file_name),
		source=sources
	)
else:
	library = env.SharedLibrary(
		"{}/libGodotStylizedShadersPlugin{}{}"
			.format(folder, env["suffix"], env["SHLIBSUFFIX"]),
		source=sources,
	)

gdextension_copy = env.Command(
	target="{}/GodotStylizedShadersPlugin.gdextension".format(folder),
	source="GodotStylizedShadersPlugin.gdextension",
	action=Copy("$TARGET", "$SOURCE")
)

# pdb_copy = env.Command(
# 	target="{}/libGodotStylizedShadersPlugin.{}.{}".format(folder, env["platform"], env["target"]),
# 	source="vc140.pdb",
# 	action=Copy("$TARGET", "$SOURCE")
# )

shutil.copytree("include/shaders/", "{}/shaders/".format(folder), dirs_exist_ok=True)

# env.AddPostAction(pdb_copy, library)
env.Depends(gdextension_copy, library)

CacheDir(".scons_cache/")

Default(library)

Default(gdextension_copy)