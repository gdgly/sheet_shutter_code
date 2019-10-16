volatile register int result asm("A");
int value;
result = __builtin_lac(value,3);