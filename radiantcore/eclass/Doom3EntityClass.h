#pragma once

#include "ieclass.h"
#include "irender.h"
#include "ifilesystem.h"

#include "math/Vector3.h"
#include "math/AABB.h"
#include "string/string.h"

#include "parser/DefTokeniser.h"

#include <vector>
#include <map>
#include <memory>

/* FORWARD DECLS */

class Shader;

namespace eclass
{

class Doom3EntityClass;
typedef std::shared_ptr<Doom3EntityClass> Doom3EntityClassPtr;

/**
 * Implementation of the IEntityClass interface. This represents a single
 * Doom 3 entity class, such as "light_moveable" or "monster_mancubus".
 */
class Doom3EntityClass
: public IEntityClass
{
    typedef std::shared_ptr<std::string> StringPtr;

    class StringCompareFunctor
    {
    public:
        bool operator()(const StringPtr& lhs, const StringPtr& rhs) const
        {
            //return boost::algorithm::ilexicographical_compare(lhs, rhs); // this is slow!
            return string_compare_nocase(lhs->c_str(), rhs->c_str()) < 0;
        }
    };

    // The name of this entity class
    std::string _name;

    // Source file information
    vfs::FileInfo _fileInfo;

    // Parent class pointer (or NULL)
    IEntityClass* _parent;

    // Should this entity type be treated as a light?
    bool _isLight;

    // Colour of this entity and flag to indicate it has been specified
    Vector3 _colour;
    bool _colourTransparent;

    // Default shader names, in case we don't get any from the parent or otherwise
    static const std::string DefaultWireShader;
    static const std::string DefaultFillShader;
    static const Vector3 DefaultEntityColour;

    // Shader versions of the colour
    std::string _fillShader;
    std::string _wireShader;

    // Does this entity have a fixed size?
    bool _fixedSize;

    // Map of named EntityAttribute structures. EntityAttributes are picked
    // up from the DEF file during parsing. Ignores key case.

    // greebo: I've changed the EntityAttributeMap key type to StringPtr, to save
    // more than 130 MB of string data used for just the keys. A default TDM installation
    // has about 780k entity class attributes after resolving inheritance.
    // However, the memory saving comes with a performance cost since we need 
    // to convert the incoming std::string queries to StringRefs before passing
    // them to std::map::find(). During a regular DarkRadiant startup 
    // there are about 64k find() operations, a single idLight
    // creation costs about 30-40 find() operations, which is ok I guess.

    typedef std::map<StringPtr, EntityClassAttribute, StringCompareFunctor> EntityAttributeMap;
    EntityAttributeMap _attributes;

    // The model and skin for this entity class (if it has one)
    std::string _model;
    std::string _skin;

    // Flag to indicate inheritance resolved. An EntityClass resolves its
    // inheritance by copying all values from the parent onto the child,
    // after recursively instructing the parent to resolve its own inheritance.
    bool _inheritanceResolved;

    // Name of the mod owning this class
    std::string _modName;

    // The empty attribute
    EntityClassAttribute _emptyAttribute;

    // Helper object to manage attached entities
    class Attachments;
    std::unique_ptr<Attachments> _attachments;

    // The time this def has been parsed
    std::size_t _parseStamp;

    // Emitted when contents are reloaded
    sigc::signal<void> _changedSignal;

private:
    // Clear all contents (done before parsing from tokens)
    void clear();
    void parseEditorSpawnarg(const std::string& key, const std::string& value);
    void setIsLight(bool val);

public:
    /**
     * Static function to create a default entity class.
     *
     * @param name
     * The name of the entity class to create.
     *
     * @param brushes
     * Whether the entity contains brushes or not.
     */
    static Doom3EntityClassPtr create(const std::string& name, bool brushes);

    /**
     * Constructor.
     *
     * @param name
     * Entity class name.
     *
     * This eclass will have isFixedSize set to false.
     */
    Doom3EntityClass(const std::string& name, const vfs::FileInfo& fileInfo);

    /**
     * Constructor.
     *
     * @param name
     * Entity class name.
     *
     * @param fixedSize
     * whether this entity has a fixed size.
     */
    Doom3EntityClass(const std::string& name, const vfs::FileInfo& fileInfo, bool fixedSize);

    virtual ~Doom3EntityClass();

    /// Add a new attribute
    void addAttribute(const EntityClassAttribute& attribute);

    // IEntityClass implementation
    std::string getName() const override;
    const IEntityClass* getParent() const override;
    sigc::signal<void>& changedSignal() override;
    bool isFixedSize() const override;
    AABB getBounds() const override;
    bool isLight() const override;
    const Vector3& getColour() const override;
    /// Set the display colour
    void setColour(const Vector3& colour) override;
    // Resets the colour to the value defined in the attributes
    void resetColour();
    const std::string& getWireShader() const override;
    const std::string& getFillShader() const override;
    EntityClassAttribute& getAttribute(const std::string& name) override;
    const EntityClassAttribute& getAttribute(const std::string& name) const override;
    void forEachClassAttribute(std::function<void(const EntityClassAttribute&)>,
                               bool) const override;

    const std::string& getModelPath() const override { return _model; }
    const std::string& getSkin() const override { return _skin; }

	bool isOfType(const std::string& className) override;

    std::string getDefFileName() override;

    /// Set a model on this entity class.
    void setModelPath(const std::string& path) {
        _fixedSize = true;
        _model = path;
    }

    /// Set the skin.
    void setSkin(const std::string& skin) { _skin = skin; }

    /**
     * Resolve inheritance for this class.
     *
     * @param classmap
     * A reference to the global map of entity classes, which should be searched
     * for the parent entity.
     */
    typedef std::map<std::string, Doom3EntityClassPtr> EntityClasses;
    void resolveInheritance(EntityClasses& classmap);

    /**
     * Return the mod name.
     */
    std::string getModName() const {
        return _modName;
    }

    /**
     * Set the mod name.
     */
    void setModName(const std::string& mn) {
        _modName = mn;
    }

    // Initialises this class from the given tokens
    void parseFromTokens(parser::DefTokeniser& tokeniser);

    void setParseStamp(std::size_t parseStamp)
    {
        _parseStamp = parseStamp;
    }

    std::size_t getParseStamp() const
    {
        return _parseStamp;
    }
};

/**
 * Pointer typedef.
 */
typedef std::shared_ptr<Doom3EntityClass> Doom3EntityClassPtr;

}
