import time

start = time.time()
sum = 0
i = 0
while i < 1000000:
    sum = sum + i
    i = i + 1

print(sum)
print("python at", (time.time() - start), "sec done.\n")
