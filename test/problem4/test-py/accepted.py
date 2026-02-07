for n in range(1,101):
    print(f"? {n}")
    response = input()
    if response == "equal":
        print(f"! {n}")
        break

