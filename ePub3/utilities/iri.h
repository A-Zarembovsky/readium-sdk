//
//  iri.h
//  ePub3
//
//  Created by Jim Dovey on 2013-01-15.
//  Copyright (c) 2012-2013 The Readium Foundation and contributors.
//  
//  The Readium SDK is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef __ePub3__iri__
#define __ePub3__iri__

#include <ePub3/utilities/utfstring.h>
#include <google-url/gurl.h>
#include <vector>

EPUB3_BEGIN_NAMESPACE

class CFI;

/**
 The IRI class encapsulates all URL and URN storage in Readium.
 
 The EPUB 3 specification calls for IRIs rather than URIs (i.e. Unicode characters
 are allowed and should not be implicitly encoded) in matching properties and other
 identifiers. This class provides URN support internally, URL support through
 Google's GURL library, and Unicode IRI support is wrapped around GURL.
 
 @ingroup utilities
 */
class IRI
{
    // would like this to contain const strings, but that proves awkward for now
    typedef std::vector<string>         ComponentList;
    typedef ComponentList::size_type    size_type;
    
    static string gPathSeparator;
    static string gURNScheme;
    static string gReservedCharacters;
    
public:
    ///
    /// A type encapsulating an account and shared-secret pair, such as username and password.
    typedef const std::pair<string, string> IRICredentials;
    
    ///
    /// The IRI scheme used to refer to EPUB 3 documents.
    static string gEPUBScheme;
    
public:
    ///
    /// Initializes an empty (and thus invalid) IRI.
    IRI() : _urnComponents(), _url(nullptr), _pureIRI() {}
    
    /**
     Create a new IRI.
     @param iriStr A valid URL or IRI string.
     */
    IRI(const string& iriStr);
    
    /**
     Create a URN.
     
     Follows the form 'urn:`nameID`:`namespacedString`'.
     @param nameID The identifier/namespace for the resource name.
     @param namespacedString The resource name.
     */
    IRI(const string& nameID, const string& namespacedString);
    
    /**
     Create a simple URL.
     
     The URL will be in the format:
     
         [scheme]://[host][path]?[query]#[fragment]
     
     If the path is empty or does not begin with a path separator character (`/`),
     one will be inserted automatically.
     @param scheme The URL scheme.
     @param host The host part of the URL.
     @param path The URL's path.
     @param query Any query components of the URL, properly URL-encoded.
     @param fragment A fragmuent used to identify a particular location within a
     resource.
     */
    IRI(const string& scheme, const string& host, const string& path, const string& query="", const string& fragment="");
    
    ///
    /// Create a copy of an existing IRI.
    IRI(const IRI& o) : _urnComponents(o._urnComponents), _url(new GURL(*o._url)), _pureIRI(o._pureIRI) {}
    
    ///
    /// C++11 move-constructor.
    IRI(IRI&& o) : _urnComponents(std::move(o._urnComponents)), _url(o._url), _pureIRI(std::move(o._pureIRI)) { o._url = nullptr; }
    
    virtual ~IRI();
    
    /// @{
    /// @name Assignment
    
    ///
    /// Assigns the value of another IRI (copy assignment).
    IRI&            operator=(const IRI& o);
    
    ///
    /// Assigns ownership of the value of another IRI (move assignment).
    IRI&            operator=(IRI&& o);
    
    /// @}
    
    /// @{
    /// @name Comparators
    
    /**
     Compares two IRIs for equality.
     @param o An IRI to compare.
     @result Returns `true` if the IRIs are equal, `false` otherwise.
     */
    bool            operator==(const IRI& o)                const;
    
    /**
     Compares two IRIs for inequality.
     @param o An IRI to compare.
     @result Returns `true` if the IRIs are *not* equal, `false` otherwise.
     */
    bool            operator!=(const IRI& o)                const;
    
    /**
     Less-than comparator.
     
     This is here to add std::sort() support to IRIs.
     @param o An IRI against which to compare.
     @result Returns `true` if `*this` is less than `o`, `false` otherwise.
     */
    bool            operator<(const IRI& o)                 const;
    
    /// @}
    
    ///
    /// Returns `true` if the IRI is a URN.
    bool            IsURN() const { return _urnComponents.size() > 1; }
    
    ///
    /// Returns `true` if the IRI is a URL referencing a relative location.
    bool            IsRelative() const { return !_url->has_host(); }
    
    /// @{
    /// @name Component Introspection
    
    ///
    /// Obtains the IRI's scheme component.
    const string    Scheme() const { return (IsURN() ? _urnComponents[0] : _url->scheme()); }    // simple, because it must ALWAYS be present (even if empty, as for pure fragment IRIs)
    
    ///
    /// Obtains the name-id component of a URN IRI.
    const string&   NameID() const { if (!IsURN()) { throw std::invalid_argument("Not a URN"); } return _urnComponents[1]; }
    
    ///
    /// Retrieves the nost component of a URL IRI.
    const string    Host() const { return _url->host(); }
    
    ///
    /// Retrieves any credentials attached to an IRI.
    IRICredentials  Credentials() const;
    
    ///
    /// Returns the namespace-qualified part of a URN IRI.
    const string    NamespacedString() const { if (!IsURN()) { throw std::invalid_argument("Not a URN"); } return _urnComponents[2]; }
    
    ///
    /// Obtains the port number associated with a URL IRI.
    int             Port() const { return _url->EffectiveIntPort(); }
    
    /**
     Obtains the path component of a URL IRI.
     @param URLEncoded If `true`, returns the path in URL-encoded format. Otherwise,
     the path will be decoded first, yielding a standard POSIX file-system path.
     */
    const string    Path(bool URLEncoded=true) const;
    
    ///
    /// Retrieves the query portion of a URL IRI, if any.
    const string    Query() const { return _url->query(); }
    
    ///
    /// Retrieves any fragment part of a URL IRI.
    const string    Fragment() const { return _url->ref(); }
    
    ///
    /// Obtains the last path component of a URL IRI.
    const string    LastPathComponent() const { return _url->ExtractFileName(); }
    
    /**
     Returns any CFI present in a URL IRI.
     
     If the fragment part of a URL is a valid Content Fragment Identifier (i.e. if
     the URL's fragment begins with `epubcfi(`) then this will parse the fragment
     into a CFI representation.
     @result A valid CFI if one is present in the URL's fragment, or an empty CFI if
     no content fragment identifier is present.
     */
    const CFI       ContentFragmentIdentifier() const;
    
    /**
     Assigns a scheme to this IRI.
     @param scheme The new scheme.
     */
    void            SetScheme(const string& scheme);
    
    /**
     Assigns a host to this IRI.
     @param host The new host component.
     */
    void            SetHost(const string& host);
    
    /**
     Sets credentials for this IRI.
     @param user The username for the credential.
     @param pass The shared-secret part of the credential.
     */
    void            SetCredentials(const string& user, const string& pass);
    
    /**
     Appends a new component to a URL IRI's path.
     @param component The new path component.
     */
    void            AddPathComponent(const string& component);
    
    /**
     Adds or replaces the query component of a URL IRI.
     @param query The new query string.
     */
    void            SetQuery(const string& query);
    
    /**
     Adds or replaces the fragment component of a URL IRI.
     @param fragment The new fragment string.
     */
    void            SetFragment(const string& query);
    
    /**
     Sets a URL IRI's fragment using a Content Fragment Identifier.
     */
    void            SetContentFragmentIdentifier(const CFI& cfi);
    
    /// @}
    
    /// @{
    /// @name Helper Methods
    
    ///
    /// URL-encodes a path, query, or fragment component.
    static string   URLEncodeComponent(const string& str);
    
    ///
    /// Percent-encodes the UTF-8 representation of any non-ASCII characters in a string.
    static string   PercentEncodeUCS(const string& str);
    
    ///
    /// Converts an IDN (hostname in non-ASCII Unicode format) into its ASCII representation.
    static string   IDNEncodeHostname(const string& host);
    
    /// @}
    
    /**
     Obtains a Unicode string representation of this IRI.
     
     Only percent-encodes URL-reserved characters within components, and uses IDN
     algorithm to obtain a Unicode hostname.
     
     Note that any components which are already URL-encoded will not be explicitly
     decoded by this function.
     @result A Unicode IRI string.
     */
    string          IRIString() const;
    
    /**
     Obtains a valid ASCII URL representation of this IRI.
     
     Percent-encodes all URL-reserved characters and all non-ASCII characters outside
     the hostname using those characters' UTF-8 representations. Uses IDN algorithm to
     obtain an ASCII representation of any Unicode hostname.
     @result An ASCII URL string; even though the result is a Unicode string type, the
     characters are guaranteed to be valid ASCII suitable for use with non-Unicode
     libraries.
     */
    string          URIString() const;
    
protected:
    ComponentList   _urnComponents;     ///< The components of a URN.
    GURL*           _url;               ///< The underlying URL object.
    string          _pureIRI;           ///< A cache of the Unicode IRI string. May be empty.
    
};

template <class _CharT, class _Traits>
inline std::basic_ostream<_CharT, _Traits>&
operator<<(std::basic_ostream<_CharT, _Traits>& __os, const IRI& iri) {
    return __os << iri.URIString();
}

EPUB3_END_NAMESPACE

#endif /* defined(__ePub3__iri__) */
