# BFS Graph Visualizer - PTIT

Dự án bài tập lớn môn Toán Rời Rạc (PTIT) - Ứng dụng mô phỏng trực quan thuật toán Duyệt theo chiều rộng (BFS) trên đồ thị.

## 🚀 Tính năng chính
- **Tương tác trực quan:** Thêm/Xóa đỉnh và cạnh trực tiếp bằng chuột.
- **Tìm đường đi:** Chọn đỉnh bắt đầu (Xanh ngọc) và đỉnh kết thúc (Đỏ tươi).
- **Trạng thái thuật toán:** Hiển thị hàng đợi (Queue) và lộ trình (Path) chi tiết ngay trên màn hình.
- **Tính năng bổ trợ:** - Hoàn tác (Undo) các thao tác trước đó bằng phím `Z`.
  - Hiệu ứng flash nút bấm (0.1s) khi thao tác.
  - Reset toàn bộ đồ thị bằng phím `R`.

## 🛠 Công nghệ sử dụng
- **Ngôn ngữ:** C++
- **Thư viện đồ họa:** SFML 3.0
- **Trình biên dịch:** MinGW (ucrt64)
- **IDE:** Visual Studio Code

## 📋 Hướng dẫn cài đặt & Chạy
1. **Yêu cầu:** Máy tính đã cài đặt thư viện SFML 3.0.
2. **Cấu hình:** Đảm bảo thư mục `.vscode` chứa file `tasks.json` đúng đường dẫn compiler của bạn.
3. **Thực thi:**
   - Mở dự án bằng VS Code.
   - Nhấn `Ctrl + Shift + B` để build.
   - Nhấn `F5` để chạy ứng dụng.
   - **Lưu ý:** File `arial.ttf` phải nằm cùng thư mục với file `.exe` để hiển thị chữ.
