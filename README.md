# Hệ thống tưới tiêu tự động (v2)

## Mở Đầu
Đây là dự án cuối kỳ cho môn học "Phát triển ứng dụng IoT" tại trường Đại học Khoa học Tự nhiên - Đại học Quốc gia Hà Nội (HUS-VNU) của tôi. Dự án được phát triển từ dự án giữa kỳ cùng tên, nhưng đã được cải tiến đôi chút.

## Những thay đổi
### Phần cứng: 
- Không có quá nhiều sự thay đổi ở phần cứng. Tôi vẫn sử dụng các linh kiện: 
  + ESP32 DOIT Devkit V1
  + L298N Motor Driver
  + Máy bơm nước mini (3-12V)
  + Cảm biến độ ẩm đất
  + 2 cell Pin Lion 18650 3.7V 20A
- Ngoài ra, tôi còn thêm module cảm biến DHT22 để theo dõi nhiệt độ - độ ẩm ở lần này

### Phần mềm:
- Đây là phần tôi có nhiều sự thay đổi và cải tiến nhất ở dự án lần này so với dự án giữa kỳ. Bao gồm:
  + Tạo một máy ảo từ một EC2 instance của AWS phục vụ việc chạy node-red server trên Internet thông qua IP công khai
  + Tạo một kho lưu trữ (Database) trên node-red server bằng MySQL phục vụ việc lưu trữ trong thời gian dài
  + Tích hợp thêm các API để giao tiếp client-server dễ dàng
  + Thêm dịch vụ gửi email cảnh báo tới người dùng khi có sự kiện
  + Tạo UI WebDashboard bằng node-red-dashboard
  + Bảo mật: Thêm chứng chỉ TLS cho server, thêm bước Authentication cho giao diện node-red flow
  + Code chính:
    * Thay đổi các khai báo biến để tiện chỉnh sửa
    * Chia các MQTT Topic riêng biệt
    * Thêm hàm DHT_22_Reader() để đọc/gửi giá trị nhiệt độ - độ ẩm không khí
    * Thêm hàm soil_Moisture_Reader() để đọc/gửi giá trị độ ẩm đất
    * Thêm hàm soil_Moisture_Handler() để xử lý dữ liệu độ ẩm đất (Vì giá trị độ ẩm đất quyết định việc tưới nước)
    * Loại bỏ hàm sensor_Handler(), mqttPublish()
    * Thay thế delay() bằng Ticker để tránh gián đoạn quá trình truyền nhận của ESP32


## Kết quả
- Sau khi áp dụng các thay đổi đã kể trên, tôi có được:
  + Giao diện Web Server:
  + Database:
  + Điều khiển từ người dùng tới hệ thống tưới tiêu:
  + Cảnh báo qua email khi có sự kiện:
  + Hoạt động của hệ thống nói chung:

## Kết Luận
- Sau khi hoàn thành dự án này, tôi đã học và làm được một số thứ liên quan tới các công nghệ:
  + PlatformIO: Môi trường phát triển cho các ứng dụng IoT
  + Node-RED: Trực quan hóa các luồng, các giao thức theo dạng lập trình kéo thả và phát triển APIs/Dashboard
  + Giao thức MQTT và các broker trong MQTT
  + Chứng chỉ TLS/SSL

    

