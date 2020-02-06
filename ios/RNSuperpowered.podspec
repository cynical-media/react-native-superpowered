
Pod::Spec.new do |s|
  s.name         = "RNSuperpowered"
  s.version      = "1.0.0"
  s.summary      = "RNSuperpowered"
  s.description  = <<-DESC
                  RNSuperpowered
                   DESC
  s.homepage     = "https://github.com/cfogelklou/react-native-superpowered"
  s.license      = "MIT"
  # s.license      = { :type => "MIT", :file => "FILE_LICENSE" }
<<<<<<< HEAD
  s.author             = { "author" => "chris.fogelklou@gmail.com" }
=======
  s.author             = { "author" => "cfogelklou@gmail.com" }
>>>>>>> f691184c81e0ba3c688b1d08c677bc8b483ace3b
  s.platform     = :ios, "8.0"
  s.source       = { :git => "https://github.com/cfogelklou/react-native-superpowered.git", :tag => "master" }
  s.source_files  = "RNSuperpowered/**/*.{h,m}"
  s.requires_arc = true
  s.frameworks = 'MediaPlayer'


  s.dependency "React"

end

  
