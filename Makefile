DEFULT_C=gcc

build:
	$(DEFULT_C) main.c -o sts 
install:build
	
	@echo "This will copy the sts excutabel to the Bin folder"
	@echo 3
	@sleep 1
	@echo 2
	@sleep 1
	@echo 1
	@sleep 1
	cp sts /bin/
