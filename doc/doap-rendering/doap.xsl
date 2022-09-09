<?xml version="1.0"?>

<!--
SPDX-FileCopyrightText: 2019 Adrien Destugues <pulkomandy@pulkomandy.tk>

SPDX-License-Identifier: CC0-1.0
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0"
    xmlns:doap="http://usefulinc.com/ns/doap#"
    xmlns:xmpp="https://linkmauve.fr/ns/xmpp-doap#"
    xmlns:schema="https://schema.org/"
    xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
    >
    <xsl:output method="html"/>
    <xsl:template match="doap:Project">
        <html>
            <head>
                <link href="doap.css" type="text/css" rel="stylesheet"/>
                <title><xsl:value-of select="doap:name"/> - XMPP implementation support</title>
            </head>
            <body>
                <xsl:choose>
                    <xsl:when test="doap:implements">
                        <table>
                            <tr>
                                <th>XMPP Extension</th>
                                <th>Implemented Version</th>
                                <th>Status</th>
                                <th>Since</th>
                                <th>Notes</th>
                            </tr>
                            <xsl:apply-templates select="doap:implements/*"/>
                        </table>
                    </xsl:when>
                    <xsl:otherwise>
                        <p class="no-info">No info about supported extensions available.</p>
                    </xsl:otherwise>
                </xsl:choose>
            </body>
        </html>
    </xsl:template>

    <xsl:template match="schema:logo">
        <img src="{@rdf:resource}" alt="logo" width="96" height="96" />
    </xsl:template>

    <xsl:template match="doap:homepage">
        <a class="button" href="{@rdf:resource}">Website</a>
    </xsl:template>

    <xsl:template match="doap:download-page">
        <a class="button" href="{@rdf:resource}">Download</a>
    </xsl:template>

    <xsl:template match="schema:documentation">
        <a class="button" href="{@rdf:resource}">Documentation</a>
    </xsl:template>

    <xsl:template match="doap:os">
        <div class="chip">
            <xsl:value-of select="."/>
        </div>
    </xsl:template>

    <xsl:template match="xmpp:SupportedXep">
        <!--
            Get the definition of the XEP that this block refers to.
            We will use that to extract the details of the latest revision and
            the one that is implemented, so that we can show that info as well
            as check if it is the latest revision that is being used.
        -->
        <xsl:variable
            name="xepnumber"
            select="substring-before(substring-after(xmpp:xep/@rdf:resource, 'https://xmpp.org/extensions/xep-'), '.')"
        />
        <xsl:variable
            name="xeplist"
            select="document('xeplist.xml')/xep-infos"
        />
        <xsl:variable
            name="xep-descriptor"
            select="$xeplist/xep[number/text() = number($xepnumber)]"
        />
        <tr>
            <td>
                <a href="{xmpp:xep/@rdf:resource}" title="{$xep-descriptor/abstract/text()}" id="xep-{$xepnumber}" target="_blank">
                    XEP-<xsl:value-of select="$xepnumber"/>
                    <xsl:if test="$xep-descriptor/title/node()">
                        <xsl:text>: </xsl:text>
                        <xsl:value-of select="$xep-descriptor/title/node()"/>
                    </xsl:if>
                </a>
            </td>
            <td>
                <span class="version">
                    <xsl:value-of select="xmpp:version"/>
                    <xsl:text> </xsl:text>
                </span>
                <xsl:if test="$xep-descriptor/last-revision/version/text() != xmpp:version">
                    <br></br>
                    <span title="{$xep-descriptor/last-revision/date/text()}" class="version version-outdated">
                        <xsl:text>latest: </xsl:text>
                        <xsl:value-of select="$xep-descriptor/last-revision/version/text()"/>
                        <xsl:text> </xsl:text>
                    </span>
                </xsl:if>
            </td>
            <td>
                <span class="xep-implementation-status {xmpp:status}"><xsl:value-of select="xmpp:status"/></span>
            </td>
            <td>
                <span class="version"><xsl:value-of select="xmpp:since"/></span>
            </td>
            <td>
                <span class="small"><xsl:value-of select="xmpp:note"/></span>
            </td>
        </tr>
    </xsl:template>
</xsl:stylesheet>
