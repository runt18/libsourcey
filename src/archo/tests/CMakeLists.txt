include_dependency(OpenSSL REQUIRED)
include_dependency(JsonCpp)

define_libsourcey_test(archotests base uv net http json archo)
