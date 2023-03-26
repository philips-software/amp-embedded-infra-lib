Feature: Http

Scenario: Serve a static page over http
    Given I start a http server
    And I add a page with static content
    When I make a http request to localhost
    Then I check that the response is correct
