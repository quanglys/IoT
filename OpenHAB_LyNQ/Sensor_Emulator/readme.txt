Chương trình giả lập các sensor gửi dữ liệu về OpenHAB thông qua RestAPI (do tạm thời em chưa biết cách gửi qua MQTT trên code C).
Chương trình sẽ đọc các thông số từ file cấu hình và thực hiện mô phỏng gửi dữ liệu về OpenHAB.
Mô tả:
  chương trình có sử dụng các API của windows nên chỉ buil và chạy được trên windows
  file cấu hình config.cfg đặt cũng file thực thi
  Cấu trúc file config:
    dòng đầu: địa chỉ cài đặt OpenHAB. VD: c:\OpenHAB
    dòng thứ 2: tên MQTT broker sử dụng trong mục MQTT transport của file cấu hình openhab.cfg
    dòng thứ 3: số lượng sensor 
    các dòng tiếp theo mỗi dòng gồm 2 giá trị nguyên: <type> <time>
      trong đó  type: loại sensor: 1 (nhiệt độ) hoặc 2 (độ ẩm)
                time: thời gian nghỉ (ms) giữa 2 lần gửi dữ liệu về server.
  Chương trình sẽ đọc file cấu hình và sinh ra các file demo.sitemap và demo.items trong thư mục cấu hình tương ứng của openHAB.
  Sau khi sinh xong file cấu hình chương trình sẽ thực hiện các giả lập gửi dữ liệu về server. Các giá trị nhiệt độ sẽ trong miền [5,50] (độ C) và độ ẩm trong miền [45,100] (%)
