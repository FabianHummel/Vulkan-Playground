.PHONY: all

all: clean shaders

clean:
	rm -rf ~/Library/Developer/Xcode/DerivedData/vulkan*

shaders:
	./compileShaders.sh