# While commands are provided for generating both Ninja, Make, and Xcode files,
# the default is to use Ninja.

all: ninja

clean:
	trash *.build

compile_commands_folder := ./.compile_commands
$(compile_commands_folder):
	mkdir $(compile_commands_folder)

# Makefile commands to be used when building without an IDE. Other commands are provided for generating Xcode build
# files.

ninja_dir := ./ninja.build

ninja: $(ninja_dir) $(compile_commands_folder)
	ninja -C $(ninja_dir)
	cp $(ninja_dir)/compile_commands.json $(compile_commands_folder)/compile_commands.json

ninja_clean: $(ninja_dir)
	ninja -C $(ninja_dir) clean

$(ninja_dir):
	mkdir $(ninja_dir)
	cd $(ninja_dir) && cmake -G Ninja ..

# Xcode build files

xcode_dir := ./xcode.build

xcode: $(xcode_dir) $(compile_commands_folder)
	cd $(xcode_dir) && xcrun -sdk macosx xcodebuild build | xcpretty -c
	cp $(xcode_dir)/compile_commands.json $(compile_commands_folder)/compile_commands.json

xcode_clean: $(xcode_dir)
	cd $(xcode_dir) && xcrun -sdk macosx xcodebuild clean | xcpretty -c

xcode_open: $(xcode_dir)
	open $(xcode_dir)/*.xcodeproj

$(xcode_dir):
	mkdir $(xcode_dir)
	cd $(xcode_dir) && cmake -G Xcode ..

# Sublime Text / Ninja

sublime_dir := ./sublime.build

sublime: $(sublime_dir) $(compile_commands_folder)
	ninja -C $(sublime_dir)
	cp $(sublime_dir)/compile_commands.json $(compile_commands_folder)/compile_commands.json

sublime_clean: $(sublime_dir)
	ninja -C $(sublime_dir) clean

sublime_open: $(sublime_dir)
	subl --project sublime.build/*.sublime-project -n

$(sublime_dir):
	mkdir $(sublime_dir)
	cd $(sublime_dir) && cmake -G "Sublime Text 2 - Ninja" ..
