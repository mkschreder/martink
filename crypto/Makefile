obj-$(CONFIG_CRYPTO) += sha256.o 
obj-$(CONFIG_CRYPTO_AES) += $(patsubst crypto/aes/%.c, crypto/aes/%.o, $(wildcard crypto/aes/*.c))

OUTDIRS += crypto/aes/
