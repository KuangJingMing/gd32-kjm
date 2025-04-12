def to_gb2312_escape(text):
    try:
        # 将输入的中文编码为GB2312
        gb2312_bytes = text.encode('gb2312')
        # 转换为C语言转义符格式
        escape_str = ''.join([f'\\x{byte:02X}' for byte in gb2312_bytes])
        return escape_str
    except UnicodeEncodeError:
        return "错误：输入的字符无法转换为GB2312编码"

# 获取用户输入
while True:
    user_input = input("请输入中文（输入'quit'退出）：")
    if user_input.lower() == 'quit':
        break
    result = to_gb2312_escape(user_input)
    print(f"C语言转义符格式: {result}\n")
