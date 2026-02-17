./main < inputs.txt > actual_outputs.txt
diff -s --suppress-common-lines actual_outputs.txt expected_outputs.txt
