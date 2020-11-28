import time
import math 
import numpy as np 

N = 64000000

print("Solution naive")

tab = [1. for _ in range(N)]

def norm(t):
    res = 0. 
    for val in t: res += math.sqrt(abs(val))
    return res

start = time.time()
res = norm(tab)
end = time.time()

print('res: {} - time: {}'.format(res,(end-start)))
print()
print('-------------------------------------------')
print()
print("Solution naive avec numpy")
tab = np.ones((N,))
start = time.time()
res = np.sum(np.sqrt(np.abs(tab)))
end = time.time()
print('res: {} - time: {}'.format(res,(end-start)))
print()
print('-------------------------------------------')
print()
print("Solution avec mapping")
def ops(val): return math.sqrt(abs(val))

tab = [1. for _ in range(N)]

def norm(t):
    t_ = map(ops,t)
    return sum(t_)

start = time.time()
res = norm(tab)
end = time.time()

print('res: {} - time: {}'.format(res,(end-start)))
print()
print('-------------------------------------------')
print()
print("Solution comprehension")
def ops(val): return math.sqrt(abs(val))

tab = [1. for _ in range(N)]
res = 0.0
def norm(t):
    t_ = [math.sqrt(abs(val)) for val in t]
    return sum(t_)

start = time.time()
res = norm(tab)
end = time.time()
print('res: {} - time: {}'.format(res,(end-start)))
print()
print('-------------------------------------------')



"""

Solution naive
res: 64000000.0 - time: 10.516560792922974

-------------------------------------------

Solution naive avec numpy
res: 64000000.0 - time: 0.7001543045043945

-------------------------------------------

Solution avec mapping
res: 64000000.0 - time: 10.501073837280273

-------------------------------------------

Solution comprehension
res: 64000000.0 - time: 12.471932888031006

-------------------------------------------

"""

"""arthur

Solution naive
res: 64000000.0 - time: 7.377558469772339

-------------------------------------------

Solution naive avec numpy
res: 64000000.0 - time: 0.6059095859527588

-------------------------------------------

Solution avec mapping
res: 64000000.0 - time: 8.128232717514038

-------------------------------------------

Solution comprehension
res: 64000000.0 - time: 9.993019342422485

-------------------------------------------

"""