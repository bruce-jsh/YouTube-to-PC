<div align="center">

<img src="../res/logo.png" width="72" alt="YouTube to PC">

# Khi Windows không cho tải hoặc cài đặt

</div>

---

**Không cảnh báo nào ở đây có nghĩa là chương trình nguy hiểm.**

Windows tin một phần mềm dựa trên danh tiếng, và danh tiếng đó mua được bằng chứng
chỉ ký. Chương trình này do một người viết và chưa có chứng chỉ, nên Windows coi nó
là người lạ và cảnh báo rất to.

Hãy tìm trường hợp của bạn bên dưới. Hầu hết chỉ cần ba lần nhấn.

---

## Cách nhanh nhất: dùng trình duyệt khác

Microsoft Edge là trình duyệt khó tính. **Chrome và Firefox tải tệp này mà không
nói gì.**

Nếu Edge gây khó, hãy mở
[trang tải xuống](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest) bằng
Chrome hoặc Firefox rồi nhấn nút ở đó. Chỉ vậy thôi.

Nếu bạn vẫn muốn dùng Edge, hãy đọc tiếp.

---

## 1. Edge chặn việc tải xuống

Một thông báo hiện ra nói rằng tệp **không được tải xuống thường xuyên**.

<img src="images/block-1-notice.png" width="380" alt="Edge báo rằng tệp không được tải xuống thường xuyên">

**Bước 1.** Nhấn `Ctrl` + `J` để mở **Tải xuống**, rồi nhấn nút **…** ở dòng đó.

<img src="images/block-2-menu.png" width="520" alt="Menu ba chấm với Giữ lại và Báo cáo tệp này an toàn">

**Bước 2.** Chọn **Giữ lại** (Keep).

**Bước 3.** Một cửa sổ thứ hai hiện ra. Đây là chỗ nhiều người mắc kẹt: nút duy
nhất nhìn thấy được ghi **Xóa** (Delete). Hãy nhấn **mũi tên nhỏ bên cạnh Xóa**
rồi chọn **Vẫn giữ lại** (Keep anyway).

<img src="images/block-3-keep.png" width="370" alt="Nhấn mũi tên cạnh Xóa để hiện Vẫn giữ lại">

Sau đó việc tải xuống hoàn tất bình thường.

> Nhân tiện đang ở menu đó, bạn nên nhấn **Báo cáo tệp này an toàn** (Report this
> file as safe). Càng nhiều người làm vậy thì cảnh báo càng sớm biến mất với mọi
> người.

---

## 2. "Windows đã bảo vệ PC của bạn" khi chạy

Một cửa sổ màu xanh có thể hiện ra, chỉ có nút **Không chạy**.

1. Nhấn liên kết nhỏ **Thông tin thêm** ngay dưới dòng chữ. Đó là chữ thường chứ
   không phải nút, rất dễ bỏ sót
2. Một nút mới hiện ra ở dưới: **Vẫn chạy**
3. Nhấn vào đó

Cửa sổ này thường xuất hiện ở lần chạy đầu tiên trên một máy. Sau khi bạn cho phép
một lần, Windows sẽ không hỏi lại trên máy đó nữa.

---

## 3. Phần mềm diệt vi-rút đã xóa tệp

Tải xong mà tệp biến mất.

1. Mở **Bảo mật Windows** từ menu Start
2. Vào **Bảo vệ khỏi vi-rút và mối đe dọa**
3. Trong **Lịch sử bảo vệ**, tìm mục `YouTubeToPC-Setup`
4. Mở ra, nhấn **Hành động** rồi chọn **Cho phép trên thiết bị**
5. Tải tệp lại

Với phần mềm khác (Avast, AVG, Norton, Kaspersky), hãy thêm ngoại lệ cho tệp cài
đặt hoặc cho thư mục cài đặt:

```
C:\Program Files\YouTube to PC
```

Hoặc tạm dừng bảo vệ thời gian thực một phút, cài xong rồi bật lại.

---

## 4. Cài đặt chạy được một nửa rồi dừng

Các thông báo về yt-dlp hoặc về việc kiểm tra kết nối nghĩa là trình cài đặt không
tải được các công cụ nó cần.

Trình cài đặt chỉ khoảng 1 MB. Trong lúc cài, nó tải yt-dlp, FFmpeg và QuickJS từ
trang chính thức của từng dự án, tổng cộng chừng 110 MB, và giữ bản sao riêng bên
trong thư mục chương trình.

Hãy thử theo thứ tự sau.

**a. Tắt VPN rồi cài lại.** Đây là nguyên nhân phổ biến nhất.

**b. Cho phép qua tường lửa.** **Bảo mật Windows** →
**Tường lửa và bảo vệ mạng** → **Cho phép ứng dụng qua tường lửa** →
**Thay đổi cài đặt** → **Cho phép ứng dụng khác** → **Duyệt**, chọn tệp cài đặt,
**Thêm**, rồi đánh dấu cả **Riêng tư** và **Công cộng**.

**c. Thử mạng khác.** Mạng công ty, trường học và một số mạng công cộng chặn
GitHub, nơi các công cụ được tải về. Mạng ở nhà hoặc phát sóng từ điện thoại
thường dùng được.

---

## 5. Hoàn toàn không có phản hồi

- **Nhấn đúp mà không có gì xảy ra.** Nhấn chuột phải vào tệp và chọn
  **Chạy với tư cách quản trị viên**.
- **"Ứng dụng này không chạy được trên PC của bạn".** Bạn cần Windows 10 hoặc 11
  bản 64-bit. Xem trong **Cài đặt**, **Hệ thống**, **Giới thiệu**.
- **Tệp có 0 byte hoặc không mở được.** Tải xuống bị đứt giữa chừng. Xóa đi và tải
  lại.

---

## Kiểm tra tệp có đúng bản gốc không

Mỗi trang phát hành đều ghi SHA-256 của trình cài đặt. Mở PowerShell trong thư mục
chứa tệp và chạy:

```powershell
Get-FileHash .\YouTubeToPC-Setup-1.3.3.exe -Algorithm SHA256
```

Nếu khớp với giá trị trên
[trang phát hành](https://github.com/bruce-jsh/YouTube-to-PC/releases/latest),
bạn đang có đúng tệp đã công bố và không ai động vào nó.

---

## Vì sao lại như vậy

Chứng chỉ ký tốn tiền mỗi năm, và nếu không có nó thì Windows không có cách nào
biết ai đã viết chương trình này.

Bù lại, mã nguồn được công khai, mọi bản phát hành đều được
[GitHub Actions](https://github.com/bruce-jsh/YouTube-to-PC/actions) biên dịch
công khai, và mã băm ở trên cho phép bạn tự xác nhận mình nhận đúng bản dựng đó.

Việc xin chữ ký đang được tiến hành. Xong việc đó thì các cảnh báo này sẽ biến mất.

---

## Vẫn chưa được

Hãy viết thư tới **[sunghunet@gmail.com](mailto:sunghunet@gmail.com)** hoặc mở một
[issue](https://github.com/bruce-jsh/YouTube-to-PC/issues). Cho tôi biết thông báo
viết gì và nó xuất hiện ở bước nào. Có ảnh chụp màn hình thì càng tốt.

<div align="center">

[Quay lại trang chính](README.vi.md)

</div>
