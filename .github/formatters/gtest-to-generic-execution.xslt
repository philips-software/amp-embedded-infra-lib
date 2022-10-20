<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
    <xsl:output method="xml" indent="yes" omit-xml-declaration="yes"/>

    <!-- Use Muenchian grouping to group by filename; https://en.wikipedia.org/wiki/XSLT/Muenchian_grouping -->
    <xsl:key name="testcases-by-file" match="*/testsuite/testcase" use="@file" />

    <xsl:template match="/">
            <xsl:for-each select="*/testsuite/testcase[count(. | key('testcases-by-file', @file)[1]) = 1]">
                <xsl:variable name="current-grouping-key" select="@file"/>
                <xsl:variable name="current-group" select="key('testcases-by-file', $current-grouping-key)"/>

                <file path="{$current-grouping-key}">
                    <xsl:for-each select="$current-group">
                        <testCase name="{@name}" duration="{1000 * number(@time)}">
                            <xsl:if test="contains(@status, 'notrun')">
                                <skipped message="Disabled">Skipped disabled test</skipped>
                            </xsl:if>
                        </testCase>
                    </xsl:for-each>
                </file>
            </xsl:for-each>
    </xsl:template>

</xsl:stylesheet>
