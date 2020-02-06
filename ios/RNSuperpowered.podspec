
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
  s.author             = { "author" => "chris.fogelklou@gmail.com" }
  s.platform     = :ios, "9.0"
  s.source       = { :git => "https://github.com/cfogelklou/react-native-superpowered.git", :tag => "master" }
  s.source_files  = "RNSuperpowered/**/*.{h,m,mm}"
  s.requires_arc = true
  s.frameworks = 'MediaPlayer'


  s.dependency "React"

end

  
