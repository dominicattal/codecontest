#!/usr/bin/python3

"""
Try each possible position (in the second half of the word onwards)
as the middle of the palindrome. Determine the required morse suffix,
then use dynamic programming to find the fewest characters required
to make that suffix.

I think it is correct to say that it will never be best for the middle
to be after the end of the morse-encoded word.

author: Finn Lidbetter
"""

from sys import stdin

WORD_LEN_MAX = 80
MAP = {
    'A': '.-',
    'B': '-...',
    'C': '-.-.',
    'D': '-..',
    'E': '.',
    'F': '..-.',
    'G': '--.',
    'H': '....',
    'I': '..',
    'J': '.---',
    'K': '-.-',
    'L': '.-..',
    'M': '--',
    'N': '-.',
    'O': '---',
    'P': '.--.',
    'Q': '--.-',
    'R': '.-.',
    'S': '...',
    'T': '-',
    'U': '..-',
    'V': '...-',
    'W': '.--',
    'X': '-..-',
    'Y': '-.--',
    'Z': '--..',
    '0': '-----',
    '1': '.----',
    '2': '..---',
    '3': '...--',
    '4': '....-',
    '5': '.....',
    '6': '-....',
    '7': '--...',
    '8': '---..',
    '9': '----.',
}
REV_MAP = {value: key for key, value in MAP.items()}
MORSE_CHAR_MAX = max(len(value) for value in MAP.values())


def is_palindrome(word):
    for index in range(len(word)):
        if word[index] != word[len(word) - index - 1]:
            return False
    return True


def main():
    word = stdin.readline().strip()
    morse = ''.join([MAP[symbol] if symbol in MAP else '' for symbol in word])
    if is_palindrome(morse):
        print(0)
        return
    best_suffix = 'A' * (len(word) * MORSE_CHAR_MAX)
    start_index_pairs = []
    for index in range(len(morse) // 2, len(morse)):
        start_index_pairs.append((index, index))
        start_index_pairs.append((index, index + 1))
    for lo_index, hi_index in start_index_pairs:
        valid = True
        while hi_index < len(morse):
            if morse[lo_index] != morse[hi_index]:
                valid = False
                break
            lo_index -= 1
            hi_index += 1
        if not valid:
            continue
        morse_suffix_chars = []
        while lo_index >= 0:
            morse_suffix_chars.append(morse[lo_index])
            lo_index -= 1
        morse_suffix = ''.join(morse_suffix_chars)
        char_suffix = solve(morse_suffix)
        if len(char_suffix) < len(best_suffix):
            best_suffix = char_suffix
    print(f'{len(best_suffix)} {best_suffix}')


def solve(morse_target):
    """
    Not as efficient as it could be.

    String concatenation makes this O(len(morse_target)^2)
    """
    dp = [None for _ in range(len(morse_target) + 1)]
    dp[0] = ''
    for i in range(len(morse_target)):
        for lookback in range(1, MORSE_CHAR_MAX + 1):
            if i + 1 - lookback >= 0 and dp[i + 1 - lookback] is not None:
                candidate = morse_target[i + 1 -lookback:i + 1]
                if candidate not in REV_MAP:
                    continue
                ch = REV_MAP[candidate]
                if dp[i + 1] is None:
                    dp[i + 1] = dp[i + 1 - lookback] + ch
                elif len(dp[i + 1 -lookback]) + 1 < len(dp[i + 1]):
                    dp[i + 1] = dp[i + 1 - lookback] + ch
                elif len(dp[i + 1 - lookback]) + 1 == len(dp[i + 1]):
                    concatenated = dp[i+1-lookback] + ch
                    if concatenated < dp[i + 1]:
                        dp[i + 1] = concatenated
    return dp[len(morse_target)]


if __name__ == '__main__':
    main()
