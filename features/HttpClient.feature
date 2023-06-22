Feature: HTTP Client
    In order to use the IoT capabilities of amp-embedded-infra-lib
    As an integrator
    I want to make HTTP calls and act on the results

Scenario: GET a page over HTTP
    Given I make a HTTP request to '3.230.204.70'
    #When I receive the HTTP response
    #Then the response status code is 200
    Then the response body contains 'http://httpbin.org/get'
