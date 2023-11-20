#include "services/network/test_doubles/Certificates.hpp"

namespace services
{
    namespace
    {
        const char testCaCertificateData[] =
            "-----BEGIN CERTIFICATE-----\r\n"
            "MIIDlTCCAn0CFC/btGDjlEejY84bSI5foC5LkLfdMA0GCSqGSIb3DQEBCwUAMIGG\r\n"
            "MQswCQYDVQQGEwJOTDEWMBQGA1UECAwNTm9vcmQtQnJhYmFudDESMBAGA1UEBwwJ\r\n"
            "RWluZGhvdmVuMRAwDgYDVQQKDAdQaGlsaXBzMR4wHAYDVQQLDBVFbmdpbmVlcmlu\r\n"
            "ZyBTb2x1dGlvbnMxGTAXBgNVBAMMEGNhLnBoaWxpcHMubG9jYWwwHhcNMjIwMTE3\r\n"
            "MTkwMDQwWhcNMzIwMTE1MTkwMDQwWjCBhjELMAkGA1UEBhMCTkwxFjAUBgNVBAgM\r\n"
            "DU5vb3JkLUJyYWJhbnQxEjAQBgNVBAcMCUVpbmRob3ZlbjEQMA4GA1UECgwHUGhp\r\n"
            "bGlwczEeMBwGA1UECwwVRW5naW5lZXJpbmcgU29sdXRpb25zMRkwFwYDVQQDDBBj\r\n"
            "YS5waGlsaXBzLmxvY2FsMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA\r\n"
            "x/F0etxVTo8q6Ayfo6ATa+rLXjIuoLDpXnhlXIglfVUHfiDWkfOXRz7H4oEPAEvQ\r\n"
            "6kkTNrxJQRJwtD8fIWLAMldZfJVlNhyfVnVDY20Bv15iKZIcDyL9DWNqMgNJxGUf\r\n"
            "IwFB2AFKN8B7LNMolSEfz/3p4K7p7REjKiUzNdXSu0lVOVlAy/ohIaBwjA+KB20p\r\n"
            "s8TFq6+coECLickMltBbZZhATi6xDBprk7n/Iazd2mX7M+D5TbB1fAbcMFsTXo+x\r\n"
            "kSVX3laK0aTEpEZ1boLxVDScUjYHw/REd0EBMzbOjLAAzHV5firPAYdVrfhHdGRG\r\n"
            "H6KPoxG454HNnQYHwGHY9QIDAQABMA0GCSqGSIb3DQEBCwUAA4IBAQAK4ooY3FWN\r\n"
            "BlJa1iH+aHLZsscSKDSan4E3vIXBxW/FHv9gBX9wBTiuV7fWk3oBH+jMQXcI9h31\r\n"
            "SC6EzpIhdzqvhp+/cGeJ4lydj6IKfERzMerakUgEHpLWkLdtC0PDXkJc8wEKJtsA\r\n"
            "e/plV1fgYrzLmco4KcOxHLnbPxQc9LM/VdQMtfroYk1RlF/UNruah+XFKBdv/Fvu\r\n"
            "H4Jz1nvIGG7FnbRIAEIZVy/usxHr53kTF/K8SYyBkZxQbMgpCqENDwqIyGwUyFGB\r\n"
            "udn7/yJBn4MkkhtPSTJ05XbEvSv/TyBdR/q/uYDz6ukAjBELzUyja25zQJqTz/j7\r\n"
            "pwiBjxQESglD\r\n"
            "-----END CERTIFICATE-----\r\n";

        const char testServerCertificateData[] =
            "-----BEGIN CERTIFICATE-----\r\n"
            "MIIDlTCCAn0CFHxLvb2lZiQzxiOCu5Asw17CLpQVMA0GCSqGSIb3DQEBCwUAMIGG\r\n"
            "MQswCQYDVQQGEwJOTDEWMBQGA1UECAwNTm9vcmQtQnJhYmFudDESMBAGA1UEBwwJ\r\n"
            "RWluZGhvdmVuMRAwDgYDVQQKDAdQaGlsaXBzMR4wHAYDVQQLDBVFbmdpbmVlcmlu\r\n"
            "ZyBTb2x1dGlvbnMxGTAXBgNVBAMMEGNhLnBoaWxpcHMubG9jYWwwHhcNMjIwMTE3\r\n"
            "MTkwMzE1WhcNMzIwMTE1MTkwMzE1WjCBhjELMAkGA1UEBhMCTkwxFjAUBgNVBAgM\r\n"
            "DU5vb3JkLUJyYWJhbnQxEjAQBgNVBAcMCUVpbmRob3ZlbjEQMA4GA1UECgwHUGhp\r\n"
            "bGlwczEeMBwGA1UECwwVRW5naW5lZXJpbmcgU29sdXRpb25zMRkwFwYDVQQDDBBl\r\n"
            "cy5waGlsaXBzLmxvY2FsMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA\r\n"
            "rcYgmszNxJu5pfLpploIw12iIfAyZWK6PZ1/6zlifaR1v0KLx08aLSQAOAvnb8jm\r\n"
            "fVzkBwpt3hYVm69gWdTJcFpRXMuZ1nIur7HEDQXtzrgvmV6vd8Eu7ngoGfHijP9e\r\n"
            "iuwxoYNiMmbyvTM2hYnRn5WllOnbK5OBldnh0Vawh/XErMO/rgDCq8qoNN15gAk0\r\n"
            "YQ06VKALW/tE7tSSgWMbtzHPrYWrvAJ7QoWOqZmNOJd3TaCgdut2mu6ZJq4XjGE2\r\n"
            "+xyA/brtD6GUXfWBY/bUNvLPtREMtIWaoAqt2CvQzJxJr4fxMW/sP88AHNMuL5V2\r\n"
            "IMnXgLqEym+PR647T61mmwIDAQABMA0GCSqGSIb3DQEBCwUAA4IBAQBqT/YeqJ0u\r\n"
            "k6fB6EUussbyfxPbGoKaYQ3ThVXxKXwEckREZ2wnXK1DfEXcrKkb0tSdQrfu8qLS\r\n"
            "vn6ahvzfSuXatZU/SXfrZJ5btM40FkVWknFq36MV2uZ4qsYcO8k+qOtpw2eSyuc1\r\n"
            "6WTtj5NJCDXvzs5rQJ+v3wgwVt+yeyoCX2LWMtCDO8Lfa5NsMZS+9HBSlK1oojMM\r\n"
            "Jd+TjhiFV64y7S3U0OxWNURBoJedrbcmh1kcRPUrB8CAjiZQr+RLFKuvRl6DJyg2\r\n"
            "PZAwlTPIYbBbZWqTMj7Inc13HDBmLA5NSUl9Q5P8IennIcAiYCe0V0yFlsqPAb8N\r\n"
            "mKvOiAaSo9El\r\n"
            "-----END CERTIFICATE-----\r\n";

        const char testServerKeyData[] =
            "-----BEGIN RSA PRIVATE KEY-----\r\n" //NOSONAR
            "MIIEogIBAAKCAQEArcYgmszNxJu5pfLpploIw12iIfAyZWK6PZ1/6zlifaR1v0KL\r\n"
            "x08aLSQAOAvnb8jmfVzkBwpt3hYVm69gWdTJcFpRXMuZ1nIur7HEDQXtzrgvmV6v\r\n"
            "d8Eu7ngoGfHijP9eiuwxoYNiMmbyvTM2hYnRn5WllOnbK5OBldnh0Vawh/XErMO/\r\n"
            "rgDCq8qoNN15gAk0YQ06VKALW/tE7tSSgWMbtzHPrYWrvAJ7QoWOqZmNOJd3TaCg\r\n"
            "dut2mu6ZJq4XjGE2+xyA/brtD6GUXfWBY/bUNvLPtREMtIWaoAqt2CvQzJxJr4fx\r\n"
            "MW/sP88AHNMuL5V2IMnXgLqEym+PR647T61mmwIDAQABAoIBAEt6jVrvrkjBoyeT\r\n"
            "lS0uWjQRjYTUQe+LrH1q1c8kA8WzE3nZu8D8eNA2nOm7MfF5/7NgD6OHQiV+zi/r\r\n"
            "QxyBsmVctY5q2Q0uV1z1B8ToWFy73+DsIacQgAQQqMpsKA06NqR85ynSWWqvEKxh\r\n"
            "fEMQk76PT3aZBRuuXFDsbIty7gZ/YIuglDVvv99gPi6CAttAhOICbWbL2isnuUZt\r\n"
            "GyQBlaMS1lIUPkhLjnEyuKCiRW/GiEq/veDe7uQn9SPnKSnI7Rxxv78c2KqRGF3o\r\n"
            "QHy3GklF74U60wSGH7tO9FOEcbYk1nqVv3CTctOK6VIGxpEB+gop8vYGGX7KJUP1\r\n"
            "mpPY9vECgYEA1QGTFqUBiOYiLQqgZNePAkPv9TjPkGmMAtBkAwP8r/1p0vU493ZX\r\n"
            "pTKyGY9bzlRxAjmYO9D2IizMeze+865JCRG2X4pNDMXCEX5LAzp9taYXOIlbyfqJ\r\n"
            "n6b1a16HsvyWNqwL0BtF1nheWOD2ZACv78nmtumzdJf0QQoKPzmcT+UCgYEA0NlZ\r\n"
            "2b356G14TLMPJjOSUHO0+OhKRnjB/eCUxV30ERGITYrcaXrfWYkPyQhfZcSxPVO0\r\n"
            "SIMX63Ozj6Bd8c2QbZtrmMhfnWunVNZqWZGr/mu7XqpBzc2PqgV8yuEp5/ApEfUl\r\n"
            "z/uuM0ko/OBiBwh0b16afPx2D71SWaSontBWdH8CgYAZ+a8uO8EMKMRSKdGVepP9\r\n"
            "+rrwdJUbT0O7tB3+ICZYb9bP2wevZccaZixB4bGYX8PIOa3O3ZSmZkApZorqvH47\r\n"
            "lOVgkUUEIbmg2H3dTuy1kiOSyW1gHCVcSsfy5/w0X4kR8bkfD7RciyE2RXjGiS7r\r\n"
            "VvvDoow306/9nnPRcf0V4QKBgG1g18jM0RxkcLOit5VnPwK5hpcxeOztg5PF0cSI\r\n"
            "DzH9P6h0yDjJ7D2FJepAY032NGkM9IdheN6MKwdWkrz3zuNImZJbpo/YeLtazDn6\r\n"
            "q9xEac4LVwlE1i/STegnAn6BbdEE1ffWNQaRE1FU5qIOUISfREOfKOnXWlCuhs0e\r\n"
            "CZlbAoGAfY/meNgJ2C+/IG1LsjPJCFHvN+FdfKRpotpE17Ssucn/9b2l5wg1zloX\r\n"
            "XfcVORfC9bmaPLjZlia7OSUPZAotJc5XV4CX94V7u2jHlXoUevBMIe4fztYQ5Fxk\r\n"
            "08fzkuVFGdpET1K7amOAYHqqVikz6xkcqVEf0wtQv8w0qZBkKOc=\r\n"
            "-----END RSA PRIVATE KEY-----\r\n";

        const char testClientCertificateData[] =
            "-----BEGIN CERTIFICATE-----\r\n"
            "MIIDajCCAlICFHxLvb2lZiQzxiOCu5Asw17CLpQWMA0GCSqGSIb3DQEBCwUAMIGG\r\n"
            "MQswCQYDVQQGEwJOTDEWMBQGA1UECAwNTm9vcmQtQnJhYmFudDESMBAGA1UEBwwJ\r\n"
            "RWluZGhvdmVuMRAwDgYDVQQKDAdQaGlsaXBzMR4wHAYDVQQLDBVFbmdpbmVlcmlu\r\n"
            "ZyBTb2x1dGlvbnMxGTAXBgNVBAMMEGNhLnBoaWxpcHMubG9jYWwwHhcNMjIwMTE3\r\n"
            "MjAzNDQzWhcNMzIwMTE1MjAzNDQzWjBcMQswCQYDVQQGEwJOTDEWMBQGA1UECAwN\r\n"
            "Tm9vcmQtQnJhYmFudDESMBAGA1UEBwwJRWluZGhvdmVuMRAwDgYDVQQKDAdQaGls\r\n"
            "aXBzMQ8wDQYDVQQDDAZjbGllbnQwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\r\n"
            "AoIBAQCo12n/Dl4ziKjkqL9qZALwUC6GF2TUfcQubXUyaRhMtckGxJxEIsh2LIzd\r\n"
            "N6f8jdtm0jQCTvK+5tCejN4FmKDe6wEFql86/Hlp0JKxl/AaOCyosX7NDaZoR3Xm\r\n"
            "kdJE89cEAn77hmnF1WR+vSv9lEjFYnC/LG0rc3QEl9RyinUnZ9rdxnFG/tWYytnC\r\n"
            "0vnQOBdVG5B++GD2zicoUd6iatntSNe3mCb3d57RXcSJH0aFlse4qQgOvLgZFF/o\r\n"
            "d5Y7AUbVz8WRNxMxNyo0PvCfXzSKGhpRW9xRHboNy68Nh8L5PAwE1YjXIWgHSLE/\r\n"
            "H/XIowyrluJzrw31kop8EFyOAMbzAgMBAAEwDQYJKoZIhvcNAQELBQADggEBALT9\r\n"
            "iW3p0i4QpB83icdFmxyzTdBkr6jmlkb2pUm1hZFUPBkCDZQjL0wAh8Og/pk5gEdz\r\n"
            "SBPZoosFPID+V2de8/zeVlrhTkgBU4htCXwKAQyeyJf2d2Xq2Tvr9NaBW77YJU24\r\n"
            "bhVER9XI1FKLDiFhZxvYX8vFwBe1V4P8Ee6r84CnrEynTD/Pms0VGofdAkmyyeDB\r\n"
            "gGdtGab4beCNoH7vaPnzi5hM9W0LS/11TOET/fV1mQLE20cwif6uWuuqicOYtkcQ\r\n"
            "f93YINRUIT+7+0Tav5CCfcua11wGsEsjPNLfivzXtxGqmD09ioynDWAoQExo7YlZ\r\n"
            "2cAoeoiFz2OYQ/C5mVU=\r\n"
            "-----END CERTIFICATE-----\r\n";

        const char testClientKeyData[] =
            "-----BEGIN RSA PRIVATE KEY-----\r\n" //NOSONAR
            "MIIEowIBAAKCAQEAqNdp/w5eM4io5Ki/amQC8FAuhhdk1H3ELm11MmkYTLXJBsSc\r\n"
            "RCLIdiyM3Ten/I3bZtI0Ak7yvubQnozeBZig3usBBapfOvx5adCSsZfwGjgsqLF+\r\n"
            "zQ2maEd15pHSRPPXBAJ++4ZpxdVkfr0r/ZRIxWJwvyxtK3N0BJfUcop1J2fa3cZx\r\n"
            "Rv7VmMrZwtL50DgXVRuQfvhg9s4nKFHeomrZ7UjXt5gm93ee0V3EiR9GhZbHuKkI\r\n"
            "Dry4GRRf6HeWOwFG1c/FkTcTMTcqND7wn180ihoaUVvcUR26DcuvDYfC+TwMBNWI\r\n"
            "1yFoB0ixPx/1yKMMq5bic68N9ZKKfBBcjgDG8wIDAQABAoIBADhDaGf9ErkbjiR8\r\n"
            "cyTx5OTN42L9wrTooApUoVxGFzngzfd6ZxRxftmaaOqC1HDdmMI/w+MNCelBoz1i\r\n"
            "dc3pmZoPN8z6hDd00MTJI0kY94LVO1SPqVCNful6/rfx2d7uXrVQW55XoUGypYVy\r\n"
            "Zole840LC8U4+bwkAgJR0hLFPtr9Ps2oZwcLeTKgV4Ebn/Sx6UzvDUBfiSnxi8sH\r\n"
            "Cn30RoNtxizXB1CRwNDKxrYDBEleiClW8EFUIdqUaJZDMRw1YlYG5N6+l9UOefHd\r\n"
            "kQTLm6VjGhFBFxyWpubqKr9tDIF7/9CNI9QaZoOiDQZmn3HOkRE2Bp/FPQGzt+yc\r\n"
            "lJHllokCgYEA349O9YaUk/QQo8NKCyrED/393aS4ASovGcaP09yipv1tEh+oOcyK\r\n"
            "VmvIIy4sQhQIRfHrr1zjOUDGWBu4uqzRLReyxEMVyDmJ3dBeOqaz19snjtuQ5y3w\r\n"
            "9dTyNAZ2+b2rz1XOBKVrOLUd09jkwqntJxPQXoiZx5twM6DDvv/pg2cCgYEAwVd0\r\n"
            "iHoEh12aUD0iHdL+zqdfmTASYcZUdKKzXB/VlIb9IEHS7I0WXUp/2DqocANh398b\r\n"
            "2uy2s7cXx0XxhDKA0BlqF1XZeeg2OYx0MEOpDC/84N0QJizHMDYqEwWRSPlx0IdZ\r\n"
            "tSedv6fHhg1kqak4JKMfE/xi+MmObwAZ1T3K1JUCgYAg4WwEkpmxtRjpbSSrxct2\r\n"
            "aq329m4VxcWNrfWjfHTzyJxNKRjN/3ZtRYb3Hahmw5l/6Bf4/9lvp0ZE2TbvRdGy\r\n"
            "JyUk1pUSkNvlIutpBvG2ksYbrdF/HvOsxHgnDLhc/PeTFBSr/ERr37+WU/U5aFK4\r\n"
            "B6MC9GZhxEvZXBDOO8d1iwKBgQCaD5QeRAxjySRzl6Md3MmJ5jj5GiMrohweLJ97\r\n"
            "YyqPhdkh4RGvyOTvRbQFmwgo6akN+Px7QHB9WAIsmgFPc+bM69Nr8M6wo2bCCLd0\r\n"
            "1hVYTszUaZK6uKeoNSDls2QHoC0fGxtSjNTYqVsF6Jozz1GbcBgNlulOkV1b+dqb\r\n"
            "3vdG2QKBgHn19HVWHLuWpiOjDBevQn8p5/gmupccyLc7acg7CJeK3ed9b06n9INM\r\n"
            "E2vBmYnelnxWEitF+jMLIJQjadLzpwW3qaffdGR/xGDj5r+MkKkNRO3OQ21wp8TQ\r\n"
            "85c0IkZ08K4UcHF/5gff0+FX/wJ7q0/HMsr2ESTf5xEucWpgaRdd\r\n"
            "-----END RSA PRIVATE KEY-----\r\n";
    }

    infra::BoundedConstString testCaCertificate{ testCaCertificateData, sizeof(testCaCertificateData) };
    infra::BoundedConstString testServerCertificate{ testServerCertificateData, sizeof(testServerCertificateData) };
    infra::BoundedConstString testServerKey{ testServerKeyData, sizeof(testServerKeyData) };
    infra::BoundedConstString testClientCertificate{ testClientCertificateData, sizeof(testClientCertificateData) };
    infra::BoundedConstString testClientKey{ testClientKeyData, sizeof(testClientKeyData) };
}
