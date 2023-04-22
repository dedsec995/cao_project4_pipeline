all:
	gcc -g -o sim *.c -lm
clean:
	rm -f sim && rm -f result.txt && rm -f output_memory_map.txt
run:
	rm -f ../logs/test.txt && rm -f sim && gcc -g -o sim *.c -lm && clear && ./sim ../programs/program4.txt ipeline > result.txt
run: export PRINT_PIPELINE = 1
compare:
	diff -iw ../cao2/program1_result.txt ../logs/test.txt
