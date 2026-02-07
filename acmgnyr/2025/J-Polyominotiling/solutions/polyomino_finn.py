"""
Implement the algorithm from Gambini, Vuillon 2007.
For a string of length n, we get time complexity O(n^2).

:author: Finn Lidbetter
"""

from sys import stdin

letter_map = {'u': 1, 'd': -1, 'r': 2, 'l': -2}
backtrack_map = {'u': 'd', 'd': 'u', 'r': 'l', 'l': 'r'}

def compute_lps_array(pattern):
    """
    Compute the Longest Proper Prefix which is also Suffix (LPS) array
    for the KMP algorithm.
    """
    m = len(pattern)
    lps = [0] * m  # Initialize LPS array with zeros
    # Length of the previous longest prefix & suffix
    length = 0
    i = 1  # Start from the second character
    while i < m:
        if pattern[i] == pattern[length]:
            length += 1
            lps[i] = length
            i += 1
        else:
            if length != 0:
                # Look for the next matching prefix
                length = lps[length - 1]
            else:
                # No matching prefix found
                lps[i] = 0
                i += 1
    return lps


def kmp_search_count(text, pattern):
    """
    Perform KMP string matching algorithm.
    Returns the index of first occurrence or -1 if not found.
    """
    if not pattern or not text:
        return -1
    n = len(text)
    m = len(pattern)
    lps = compute_lps_array(pattern)
    i = 0  # Index for text
    j = 0  # Index for pattern
    count = 0
    while i < n:
        if pattern[j] == text[i]:
            i += 1
            j += 1
        if j == m:
            count += 1
            j = lps[j - 1]
        elif i < n and pattern[j] != text[i]:
            if j != 0:
                j = lps[j - 1]
            else:
                i += 1
    return count

def count_tilings(word):
    word_arr = [letter_map[ch] for ch in word]
    n = len(word)
    count = 0
    for i in range(1, n):
        if word_arr[0] + word_arr[i] == 0:
            x1 = 0
            y1 = 1
            x2 = i
            y2 = i + 1
            while word_arr[(x1 - 1 + n) % n] + word_arr[y2 % n] == 0:
                x1 -= 1
                if x1 < 0 :
                    x1 += n
                y2 += 1
                if y2 >= n:
                    y2 -= n
            while word_arr[y1 % n] + word_arr[(x2 - 1 + n) % n] == 0:
                y1 += 1
                if y1 >= n:
                    y1 -= n
                x2 -= 1
                if x2 < 0:
                    x2 += n
            u_start = y1
            u_end = (x2 - 1 + n) % n
            if u_start <= u_end:
                u = word[u_start:u_end + 1]
            else:
                u = word[u_start:] + word[:u_end + 1]
            v_start = y2
            v_end = (x1 - 1 + n) % n
            if v_start <= v_end:
                v = word[v_start:v_end + 1]
            else:
                v = word[v_start:] + word[:v_end + 1]
            if x1 < y1:
                x = word[x1:y1]
            else:
                x = word[x1:] + word[:y1]
            if x2 < y2:
                y = word[x2:y2]
            else:
                y = word[x2:] + word[:y2]
            if len(u) != len(v):
                continue
            vv = v + v
            u_backtrack = ''.join([backtrack_map[ch] for ch in u[::-1]])
            if v == u_backtrack:
                # It turns out that this check is sufficient for the xy case.
                # Proving this is not so easy.
                # Add 4 for the pseudo squares.
                count += 4

                # Subtract 12 for the pseudo hexagons that are actually pseudo-squares.
                count -= 12
            # Handle the xyz factorization case.
            count += 6 * kmp_search_count(vv, u_backtrack)
    return count


def backtrack(word):
    result = []
    for ch in word:
        result.append(backtrack_map[ch])
    result = reversed(result)
    return ''.join(result)


def main():
    _, word = stdin.readline().strip().split(' ')
    print(count_tilings(word))


if __name__ == "__main__":
    main()
