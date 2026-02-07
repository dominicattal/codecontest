l, r = 1, 100
while l <= r:
    m = l + (r-l)//2
    print(f"? {m}")
    response = input()
    if response == "higher":
        l = m + 1
    elif response == "lower":
        r = m - 1
    else:
        print(f"! {m}")
        break
