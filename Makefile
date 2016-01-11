all:
	gcc dberror.c storage_mgr.c test_assign1_1.c -o test1_assign1
	./test1_assign1

test_case2:
	gcc dberror.c storage_mgr.c test_assign1_2.c -o test2_assign1
	./test2_assign1

clean:
	$(RM) test1_assign1
	$(RM) test2_assign1
