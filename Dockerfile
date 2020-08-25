FROM alpine:latest as builder
COPY . /src
WORKDIR /src
RUN apk update && apk add libc-dev gcc make curl-dev curl-static openssl openssl-libs-static nghttp2-static zlib-static
ENV CC="gcc -static"
RUN make -f Makefile clean all && make -f Makefile test

#FROM gcr.io/distroless/base-debian10
FROM alpine:latest
RUN apk add --no-cache ca-certificates
COPY --from=builder /src/example .
ENTRYPOINT ["/example"]



